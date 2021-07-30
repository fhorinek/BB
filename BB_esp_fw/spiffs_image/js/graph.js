'use strict';

var AXIS_OVER = 5

var PADDING_TOP = 20;
var PADDING_BOTTOM = 20;
var PADDING_LEFT = 90;
var PADDING_RIGHT = 200;
var PADDING = PADDING_LEFT + PADDING_RIGHT;
var OFFSET_BOTTOM = 20

var POINT_RADIUS = 5;

var LEGEND_PAD = 40;
var LEGEND_H = 20;
var LEGEND_X = PADDING_LEFT + LEGEND_PAD;
var LEGEND_Y = PADDING_TOP + LEGEND_PAD;

var PREV_OFFSET_BOTTOM = OFFSET_BOTTOM + 20;

var GRAD_ID = 0;

function hsl_from_freq(freq)
{
    if (freq == 0)
        return "url(#no-sound)";

    var h = 240 * (1 - (Math.min(freq, F_MAX)) / F_MAX)
    return "hsl(" + h + ", 100%, 50%)";
}

function create_gradient(start, stop, orig_id)
{
    if (start == 0 && stop == 0)
        return "no-sound";

    var id = orig_id + "_grad";
    GRAD_ID++;

    var grad = document.getElementById(id);
    if (grad == null)
    {
        grad = document.createElementNS("http://www.w3.org/2000/svg", "linearGradient");
        grad.setAttribute("id", id);
        grad.setAttribute("x2", 1);
        document.getElementById("svg-defs").appendChild(grad);
    }
    

    for (var m = 0; m <= 1.0; m += 0.1)    
    {
        var color = hsl_from_freq(start + (stop - start) * m)
    
        var stop_id = id + "_stop_" + Math.round(m * 10);
        
        var s = document.getElementById(stop_id);
        if (s == null)
        {
            s = document.createElementNS("http://www.w3.org/2000/svg", "stop");
            s.setAttribute("offset", Math.round(m * 100) + "%");
            s.setAttribute("id", stop_id);
            document.getElementById(id).appendChild(s);    
        }
 
        s.setAttribute("stop-color", color);
   }
    
    return id;
}

function freq_to_y(freq, h)
{
    h -= PADDING_TOP + PADDING_BOTTOM;
    return (freq * h / F_MAX) + PADDING_BOTTOM;
}

function y_to_freq(y, h)
{
    h -= PADDING_TOP + PADDING_BOTTOM;
    y -= PADDING_BOTTOM;
    var val = Math.max((y * F_MAX) / h, 0);
    
    return Math.min(F_MAX, val);
}

function dura_to_y(dura, h)
{
    return (dura * h / DURA_SCALE);
}

function y_to_dura(y, h)
{
    return Math.max(20, (y * DURA_SCALE) / h);
}

function climb_to_x(climb, w)
{
    return PADDING_LEFT + (climb + CLIMB_STEPS) * (w - PADDING) / CLIMB_DELTA;
}

function x_to_climb(x, w)
{
    var climb = Math.min(((x - PADDING_LEFT) / (w - PADDING) * CLIMB_DELTA) - CLIMB_STEPS, CLIMB_STEPS);
    return Math.max(climb, -CLIMB_STEPS);
}

function insert_points(poly, svg, points)
{
    for (var point in points)
    {
        var s = svg.createSVGPoint();
        s.x = points[point][0];
        s.y = points[point][1];
        
        poly.points.appendItem(s);
    }
}


function draw_tones(graph, tone, offset, id)
{
    var w = graph.width();
    var h = graph.height();

    var prev = false;
    var line_top_points = [];    
    var line_bottom_points = [];    

    var climbs = sorted_list(Object.keys(tone));
    
    var i;
    for (i in climbs)
    {
        var climb = climbs[i];
        
        var c2 = parseFloat(climb);
        var f2 = tone[climb][0];
        var d2 = tone[climb][1];

        var y2 = h -dura_to_y(d2, h);
        
        var x2 = Math.round(climb_to_x(c2, w));

        var y2t = y2 - offset;
        var y2b = h - offset;        

        var zero_left = false;
        var zero_right = false;

        if (prev !== false)
        {
            var c1 = parseFloat(prev);
            var f1 = tone[prev][0];
            var d1 = tone[prev][1];
            
            var y1 = h - dura_to_y(d1, h);
          
            var x1 = Math.round(climb_to_x(c1, w));
            
            var y1t = y1 - offset;
            var y1b = h - offset;

            if (f1 ==0 && d1 == 0)
            {
                zero_right = true;   
            }
            else if(f2 ==0 && d2 == 0)
            {
                zero_left = true;
            }
            else
            {
                var poly_id = "poly_" + id + "_" + i;
                var poly = document.getElementById(poly_id) ;
                if (poly == null)
                {
                    poly = document.createElementNS("http://www.w3.org/2000/svg", "polygon");
                    poly.setAttribute("id", poly_id);
                    graph.append(poly);
                }
                
                poly.style.fill = "url(#" + create_gradient(f1, f2, poly_id) + ")";
                poly.points.clear();
                
                var points = [[x1, y1t], [x2, y2t], [x2, y2b], [x1, y1b]];
                
                insert_points(poly, graph[0], points);
                
            }
        }

        line_top_points.push([x2, y2t]);
        line_bottom_points.push([x2, y2b]);

        
        var y1_left = y1
        var y2_right = y2
        
        if (zero_left)
            y1_left = 0;

        if (zero_right)
            y2_right = 0;
            
        prev = climb;
    }
    
    var outline_id = "poly_outline_" + id;
    var poly = document.getElementById(outline_id);
    if (poly == null)
    {
        poly = document.createElementNS("http://www.w3.org/2000/svg", "polygon");
        poly.classList.add("outline");
        poly.setAttribute("id", outline_id);
        poly.tone_id = id;
        graph.append(poly);    

        $(poly).click(function(e) {
            select_tone(e.target.tone_id);
        });
    }
    
    
    var line_points = line_top_points.concat(line_bottom_points.reverse());
    poly.points.clear();    
    insert_points(poly, graph[0], line_points);
    
    $(poly).dblclick(function(e){
        var w = $("#profile_dura").width();
        var h = $("#profile_dura").height();
    
        add_point(e, w, h)
    });
    
    if (id == profile_tone_selected)
    {
        var i;
        for (i in line_top_points)
        {
            var circle = document.getElementById("dura_circle_" + i);
            if (circle == null)
            {
                circle = document.createElementNS("http://www.w3.org/2000/svg", "circle");
                circle.setAttribute("id", "dura_circle_" + i);
                graph.append(circle);         
                circle.setAttribute("r", POINT_RADIUS);
                
                $(circle).mousedown(function(e){
                    profile_point_drag = e.target;
                    profile_point_wheel = e.target;
                });            

                $(circle).on("wheel", function(e){
                    profile_point_wheel = e.target;

                    var id_index = parseInt(profile_point_wheel.tone_index);
                    var sub_index = parseInt(profile_point_wheel.sub_index);
                    point_move_y(sub_index, id_index, e.originalEvent.deltaY);
                });            

                
                $(circle).dblclick(function(e){
                    var id_index = parseInt(e.target.tone_index);
                    remove_point(id_index); 
                });

            }
            circle.tone_index = i;
            circle.sub_index = 1;
            circle.setAttribute("cx", line_top_points[i][0]);
            circle.setAttribute("cy", line_top_points[i][1]);
        }
        
    }
}


function draw_axis_freq(graph)
{
    var w = graph.width();
    var h = graph.height();

    for (var climb = -CLIMB_STEPS; climb <= CLIMB_STEPS; climb++)
    {
        var poly = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
        if (climb == 0)
            poly.classList.add("center");        
        else
            poly.classList.add("axis");
        var x = climb_to_x(climb, w);
        var y1 = 0;
        var y2 = h;
        var points = [[x, y1], [x, y2]];
        
        insert_points(poly, graph[0], points);
        graph.append(poly);     
    }

    for (var freq = 0; freq <= F_MAX; freq += 500)
    {
        var poly = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
        poly.classList.add("axis");
        
        var x1 = PADDING_LEFT - AXIS_OVER;
        var x2 = w - PADDING_RIGHT + AXIS_OVER;
        var y = h - freq_to_y(freq, h);
        var points = [[x1, y], [x2, y]];
        
        insert_points(poly, graph[0], points);
        graph.append(poly);     
        
        var text = document.createElementNS("http://www.w3.org/2000/svg", "text");
        text.classList.add("freq");
        graph.append(text);   
        
        text.setAttribute("x", x1);
        text.setAttribute("y", y);

        text.textContent = Math.floor(freq) + " Hz";        
    }    
}    
    
    
function draw_axis_dura(graph)
{
    var w = graph.width();
    var h = graph.height();

    for (var climb = -CLIMB_STEPS; climb <= CLIMB_STEPS; climb++)
    {
        var poly = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
        if (climb == 0)
            poly.classList.add("center");        
        else
            poly.classList.add("axis");
        var x = climb_to_x(climb, w);
        var y1 = 0;
        var y2 = h;
        var points = [[x, y1], [x, y2]];
        
        insert_points(poly, graph[0], points);
        graph.append(poly);     
    }

    var DURA_STEP = 250;
    var dura_step = dura_to_y(DURA_STEP, h);

    for (var pos = h - OFFSET_BOTTOM; pos > 0; pos = pos - dura_step)
    {
        var poly = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
        poly.classList.add("axis");
        
        var x1 = PADDING_LEFT - AXIS_OVER;
        var x2 = w - PADDING_RIGHT + AXIS_OVER;
        var y = pos;
        var points = [[x1, y], [x2, y]];
        
        insert_points(poly, graph[0], points);
        graph.append(poly);     
        
        var text = document.createElementNS("http://www.w3.org/2000/svg", "text");
        text.classList.add("freq");
        graph.append(text);   
        
        text.setAttribute("x", x1);
        text.setAttribute("y", y);

        if (pos == h - OFFSET_BOTTOM)
            text.textContent = "0 ms";              
        else
            text.textContent = "+" + DURA_STEP + " ms";              
    }    
}

function draw_legend(graph)
{
    var LEGEND_W = graph.width() - PADDING - LEGEND_PAD * 2;

    var rect_id = "freq_legend";
    var rect = document.getElementById(rect_id);
    if (rect == null)
    {
        rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
        rect.setAttribute("id", rect_id);
        rect.classList.add("outline");
        graph.append(rect);   
        rect.setAttribute("fill", "url(#" + create_gradient(1, F_MAX,"legend") + ")");       
        
        rect.setAttribute("x", LEGEND_X);
        rect.setAttribute("y", LEGEND_Y);
        rect.setAttribute("width", LEGEND_W);
        rect.setAttribute("height", LEGEND_H);

        for (var m = 0; m <= 1; m += 1 / 4)
        {
            var text = document.createElementNS("http://www.w3.org/2000/svg", "text");
            text.classList.add("label");
            graph.append(text);   
            
            text.setAttribute("x", LEGEND_X + LEGEND_W * m);
            text.setAttribute("y", LEGEND_Y + LEGEND_H + 5);

            text.textContent = Math.floor(F_MAX * m) + " Hz";
        }
    }
}

function draw_graph(graph, tones)
{
    var h = graph.height();

    var line = document.getElementById("prev_ruler_dura");
    if (line == null)
    {
        line = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
        line.setAttribute("id", "prev_ruler_dura");
        line.classList.add("prev_ruler");
        graph.append(line);     
    }

    var offset = OFFSET_BOTTOM;
    for (var index in tones)
    {
        var max_height = get_max_height(tones[index]);
        draw_tones(graph, tones[index], offset, index);
        
        profile_offsets[index] = offset;
        offset += (max_height * h / DURA_SCALE) + 10;
    }
}

function draw_tone_prev(graph, tones, climb)
{
    var w = graph.width();
    var h = graph.height();

    var line = document.getElementById("prev_ruler_dura");
    line.points.clear();
    var x = climb_to_x(climb, w);
    insert_points(line, graph[0], [[x, 0], [x, h]]);

    line = document.getElementById("prev_ruler_freq");
    line.points.clear();
    x = climb_to_x(climb, w);
    insert_points(line, graph[0], [[x, 0], [x, h]]);

    var offset = PREV_OFFSET_BOTTOM;
    var time = 0;

    var post_data = {};
    var cnt = 0;

    for (var index in tones)
    {
        var tone = tones[index];

        if (tone[1] > 0)
        {
            post_data["tone_" + cnt] = Math.round(tone[0]);
            post_data["dura_" + cnt] = Math.round(tone[1]);
            cnt++;
        }

        var rect_id = "tone_prev_box" + index;
        var rect = document.getElementById(rect_id);
        if (rect == null)
        {
            rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
            rect.setAttribute("id", rect_id);
            rect.classList.add("outline");
            graph.append(rect);   
        }
        
        var y = dura_to_y(tone[1], h);
        offset += y;
        
        
        var bar_x = w - (PADDING_RIGHT * 3 / 5);
        var bar_y = h - offset;
        var bar_w = PADDING_RIGHT / 5;
        var bar_h = y;
        
        rect.setAttribute("x", bar_x);
        rect.setAttribute("y", bar_y);
        rect.setAttribute("width", bar_w);
        rect.setAttribute("height", bar_h);
        rect.setAttribute("fill", hsl_from_freq(tone[0]));   
        
        var text_id = "tone_prev_freq_" + index;
        var text = document.getElementById(text_id);
        if (text == null)
        {
            text = document.createElementNS("http://www.w3.org/2000/svg", "text");
            text.setAttribute("id", text_id);
            text.classList.add("freq");
            graph.append(text);   
        }
        
        var text_x = bar_x - 5;
        var text_y = bar_y + bar_h / 2;
        
        text.setAttribute("x", text_x);
        text.setAttribute("y", text_y);

        if (tone[0] == 0)
            text.textContent = "pause";
        else
            text.textContent = Math.floor(tone[0]) + " Hz";
            
        if (bar_h == 0)
            text.textContent = "";
        
        text_id = "tone_prev_dura_" + index;
        text = document.getElementById(text_id);
        if (text == null)
        {
            text = document.createElementNS("http://www.w3.org/2000/svg", "text");
            text.setAttribute("id", text_id);
            text.classList.add("dura");
            graph.append(text);   
        }
        
        var text_x = bar_x + bar_w + 5;
        var text_y = bar_y;
        
        text.setAttribute("x", text_x);
        text.setAttribute("y", text_y);

        time += tone[1];

        if (bar_h != 0)
            text.textContent = Math.floor(time) + " ms";
        else            
            text.textContent = "";
        
    }

    if (cnt == 0)
    {
        post_data["tone_" + cnt] = 0;
        post_data["dura_" + cnt] = 10;    
        cnt++;
    }
    post_data["cnt"] = cnt;
    
    if ($('#prev_sound').prop('checked'))
    {
        send_ajax("api/sound", post_data);
    }
    
    text_id = "tone_prev_dura_zero";
    text = document.getElementById(text_id);
    if (text == null)
    {
        text = document.createElementNS("http://www.w3.org/2000/svg", "text");
        text.setAttribute("id", text_id);
        text.classList.add("dura");
        graph.append(text);   
        text.setAttribute("x", text_x);
        text.setAttribute("y", h - PREV_OFFSET_BOTTOM);
        text.textContent = "0 ms";
    }
    
    text_id = "tone_prev_climb";
    text = document.getElementById(text_id);
    if (text == null)
    {
        text = document.createElementNS("http://www.w3.org/2000/svg", "text");
        text.setAttribute("id", text_id);
        graph.append(text);   
        text.setAttribute("x", bar_x + bar_w / 2);
        text.setAttribute("y", h - OFFSET_BOTTOM);
    }    
    text.textContent = climb.toFixed(1) + " m/s";
}

function draw_line(graph, tone)
{
    var w = graph.width();
    var h = graph.height();
    
    var line = document.getElementById("prev_ruler_freq");
    if (line == null)
    {
        line = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
        line.setAttribute("id", "prev_ruler_freq");
        line.classList.add("prev_ruler");
        graph.append(line);     
    }    
    
    var line = document.getElementById("freq_line");
    if (line == null)
    {
        line = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
        line.setAttribute("id", "freq_line");
        graph.append(line);         
    }
    
    var points = [];
    
    var climbs = sorted_list(Object.keys(tone));
    for (var i in climbs)
    {
        var climb = climbs[i];
        
        var c = parseFloat(climb);
        var f = tone[climb][0];
                
        var x = climb_to_x(c, w);
        var y = h - freq_to_y(f, h);
        
        points.push([x, y]);
    }

    line.points.clear();    
    insert_points(line, graph[0], points);
    
    $(line).dblclick(function(e){
        var w = $("#profile_freq").width();
        var h = $("#profile_freq").height();
    
        add_point(e, w, h)
            
    });
    
    var i;
    for (i in points)
    {
        var circle = document.getElementById("freq_circle_" + i);
        if (circle == null)
        {
            circle = document.createElementNS("http://www.w3.org/2000/svg", "circle");
            circle.setAttribute("id", "freq_circle_" + i);
            graph.append(circle);         
            circle.setAttribute("r", POINT_RADIUS);
            
            $(circle).mousedown(function(e){
                profile_point_drag = e.target;
                profile_point_wheel = e.target;
            });            

            $(circle).on("wheel", function(e){
                profile_point_wheel = e.target;
                
                var id_index = parseInt(profile_point_wheel.tone_index);
                var sub_index = parseInt(profile_point_wheel.sub_index);
                point_move_y(sub_index, id_index, e.originalEvent.deltaY);
            });            
            
            $(circle).dblclick(function(e){
                var id_index = parseInt(e.target.tone_index);
                
                remove_point(id_index);
            });            
            

        }
        circle.tone_index = i;
        circle.sub_index = 0;
        circle.setAttribute("cx", points[i][0]);
        circle.setAttribute("cy", points[i][1]);
    }

}

function draw_freq(graph, tone)
{
    draw_line(graph, tone)
}

