'use strict';

var CLIMB_STEPS = 10;
var CLIMB_DELTA = CLIMB_STEPS * 2;
var DURA_SCALE = 3000;

var GRAD_ID = 0
var F_MAX = 2000;

var PADDING_TOP = 20;
var PADDING_BOTTOM = 20;
var PADDING_LEFT = 90;
var PADDING_RIGHT = 200;
var PADDING = PADDING_LEFT + PADDING_RIGHT;
var OFFSET_BOTTOM = 20

var POINT_RADIUS = 5;

var profile_tones = [];
var profile_tone;

profile_tone = []
profile_tone[-10] = [350, 250]
profile_tone[0] = [500, 250]
profile_tones.push(profile_tone);

profile_tone = []
profile_tone[-10] = [0, 100]
profile_tone[0] = [0, 100]
profile_tones.push(profile_tone);   

profile_tone = [];
profile_tone[-10] = [200, 600];
profile_tone[-4.5] = [250, 600];
profile_tone[-4] = [263, 600];
profile_tone[0] = [500, 600];
profile_tone[1] = [600, 320];
profile_tone[3.5] = [1075, 100];
profile_tone[10] = [1600, 95];
profile_tones.push(profile_tone);

profile_tone = [];
profile_tone[-10] = [0, 1000];
profile_tone[-0.1] = [0, 1000];
profile_tone[0] = [0, 600];
profile_tone[0.5] = [0, 420];
profile_tone[1] = [0, 320];
profile_tone[1.5] = [0, 265];
profile_tone[2] = [0, 230];
profile_tone[10] = [0, 95];
profile_tones.push(profile_tone);

var profile_tone_selected = 0;
var profile_point_drag = null;
var profile_point_wheel = null;
var profile_offsets = [];

function sorted_list(list)
{
    var new_list = [];
    for (var i in list)
        new_list.push(parseFloat(list[i]));
        
    return new_list.sort(function(a, b) {
        return a - b;
    });
}


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

function select_tone(tone_id)
{
    profile_tone_selected = tone_id;
    profile_point_wheel = null;

    $("[id^=freq_circle_]").remove();
    $("[id^=dura_circle_]").remove();
    
    draw_freq($("#profile_freq"), profile_tones[tone_id]);
    draw_graph($("#profile_dura"), profile_tones);


    for (var i = 0; i < profile_tones.length; i++)
    {
        var outline = document.getElementById("poly_outline_" + i);
        if (i == tone_id)
            outline.classList.add("selected");
        else
            outline.classList.remove("selected");
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

function get_between(tones, key)
{
    var freq = 0;
    var dura = 0;
    var prev = false;
    var next = false;
    
    if (key in tones)
        return tones[key];
    
    var climbs = sorted_list(Object.keys(tones));
    for (var climb in climbs)
    {
        climb = climbs[climb];
    
        if (climb < key)
        {
            prev = climb;
        }
        if (climb > key)
        {
            next = climb;
            break;
        }
    }
    
    if (prev !== false && next !== false)
    {
        var m = (key - prev) / (next - prev);
        freq = tones[prev][0] + m * (tones[next][0] - tones[prev][0])
        dura = tones[prev][1] + m * (tones[next][1] - tones[prev][1])        
    }
    
    return [freq, dura];
}

function get_max_height(tones)
{
    var max_y = 0;
    for (var climb in tones)
        max_y = Math.max(max_y, tones[climb][1]);
    
    return max_y;
}

var AXIS_OVER = 5

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

var LEGEND_PAD = 40;
var LEGEND_H = 20;
var LEGEND_X = PADDING_LEFT + LEGEND_PAD;
var LEGEND_Y = PADDING_TOP + LEGEND_PAD;

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

var ajax_block = false;
var ajax_next_data = false;

function send_ajax(url, post_data)
{
    if (!ajax_block)
    {
        ajax_block = true;

        $.post({
            url: url, 
            data: post_data,
            complete: function(){
                ajax_block = false;
                if (ajax_next_data)
                {
                    send_ajax(url, ajax_next_data);
                    ajax_next_data = false;
                }
            }
        });
    }
    else
    {
        ajax_next_data = post_data;
    }

}

var PREV_OFFSET_BOTTOM = OFFSET_BOTTOM + 20;

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

function remove_point(id_index)
{
    var sorted = sorted_list(Object.keys(profile_tones[profile_tone_selected]));
    var old_key = sorted[id_index];
    
    if (sorted.length == 2)
    {
        //remove line
        delete profile_tones[profile_tone_selected];
        
        var new_tones = [];
        for (var j in profile_tones)
        {
            new_tones.push(profile_tones[j]);
        }
        profile_tones = new_tones;
        profile_tone_selected = (profile_tone_selected + 1) % profile_tones.length;
    }
    else
    {
        //remove point
        delete profile_tones[profile_tone_selected][old_key];
    }


    //clear points        
    $("[id^=poly_]").remove();
    $("[id^=tone_prev_]").remove();
    
    //reselect
    select_tone(profile_tone_selected);
}

function add_point(e, w, h)
{
    var x = e.offsetX;
    var y = e.offsetY;

    var climb = x_to_climb(x, w);
    var fd = get_between(profile_tones[profile_tone_selected], climb);
    var freq = fd[0];
    var dura = fd[1];

    if (!(climb in profile_tones[profile_tone_selected]))
    {
        $("[id^=poly_outline_]").remove();
        profile_tones[profile_tone_selected][climb] = [freq, dura];
        //reselect
        select_tone(profile_tone_selected);
    }
}

function point_move_y(sub_index, point_index, delta)
{
    var sorted = sorted_list(Object.keys(profile_tones[profile_tone_selected]));
    var key = sorted[point_index];

    var val = profile_tones[profile_tone_selected][key][sub_index];
    
    if (delta < 0)
        val--;
    else
        val++;
        
    if (sub_index == 0)
        val = Math.min(val, F_MAX);
    if (sub_index == 1)
        val = Math.min(val, DURA_SCALE);
    val = Math.max(0, val);
    
    profile_tones[profile_tone_selected][key][sub_index] = val;

    draw_line($("#profile_freq"), profile_tones[profile_tone_selected]);      
    draw_graph($("#profile_dura"), profile_tones);   
    
    var res = [];
    for (var tone in profile_tones)
    {
        tone = profile_tones[tone];
        res.push(get_between(tone, key));
    }

    draw_tone_prev($("#profile_dura"), res, key);    
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

//********************************
function point_move_x(sub_index, tone_index, diff)
{
    var sorted = sorted_list(Object.keys(profile_tones[profile_tone_selected]));
    var old_key = sorted[tone_index];

    var prev_key = sorted[tone_index - 1];
    var next_key = sorted[tone_index + 1];            

    var freq = profile_tones[profile_tone_selected][old_key][0];
    var dura = profile_tones[profile_tone_selected][old_key][1];

    var climb = old_key + diff;

    if (prev_key != undefined)
        if (prev_key >= climb - 0.01)
            climb = prev_key + 0.01;

    if (next_key != undefined)
        if (next_key <= climb + 0.01)
            climb = next_key - 0.01

    delete profile_tones[profile_tone_selected][old_key];
    profile_tones[profile_tone_selected][climb] = [freq, dura];
  

    draw_line($("#profile_freq"), profile_tones[profile_tone_selected]);      
    draw_graph($("#profile_dura"), profile_tones);
    
        var res = [];
    for (var tone in profile_tones)
    {
        tone = profile_tones[tone];
        res.push(get_between(tone, climb));
    }

    
    draw_tone_prev($("#profile_dura"), res, climb);    

    return climb;
}

var drag_climb_orig = false;
var drag_val_orig = false;

function point_drag(sub_index, tone_index, climb, val, e)
{
    if (drag_climb_orig === false && drag_val_orig === false)
    {
        drag_climb_orig = climb;
        drag_val_orig = val;
    }

    if (e.shiftKey)
        climb = drag_climb_orig;

    if (e.ctrlKey)
        val = drag_val_orig;

    var sorted = sorted_list(Object.keys(profile_tones[profile_tone_selected]));
    var old_key = sorted[tone_index];

    var prev_key = sorted[tone_index - 1];
    var next_key = sorted[tone_index + 1];            

    var freq = profile_tones[profile_tone_selected][old_key][0];
    var dura = profile_tones[profile_tone_selected][old_key][1];

    if (prev_key != undefined)
        if (prev_key >= climb - 0.01)
            climb = prev_key + 0.01;

    if (next_key != undefined)
        if (next_key <= climb + 0.01)
            climb = next_key - 0.01

    delete profile_tones[profile_tone_selected][old_key];
    profile_tones[profile_tone_selected][climb] = [freq, dura];
    profile_tones[profile_tone_selected][climb][sub_index] = val;

    draw_line($("#profile_freq"), profile_tones[profile_tone_selected]);      
    draw_graph($("#profile_dura"), profile_tones);
    
    return climb;
}

//tone
//[climb] freq, duration

$(function() {



    var dura = $("#profile_dura");
    var freq = $("#profile_freq");
    
    draw_axis_dura(dura);
    draw_legend(dura);    
    draw_graph(dura, profile_tones);

    draw_axis_freq(freq);
    draw_freq(freq, profile_tones[profile_tone_selected]);
    
    select_tone(profile_tone_selected);
    
    $(dura).mousemove(function(e) {
        var x = e.offsetX;
        var y = e.offsetY;

        var w = $("#profile_dura").width();
        var h = $("#profile_dura").height();
    
        var climb = x_to_climb(x, w);
        
        if (profile_point_drag != null)
        {
            var dura = Math.max(0, y_to_dura(h - y - profile_offsets[profile_tone_selected], h));
            
            var freq = y_to_freq(h - y, h);
            var tone_index = parseInt(profile_point_drag.tone_index);
            var sub_index = parseInt(profile_point_drag.sub_index);            
            
            if (sub_index == 1)
                climb = point_drag(sub_index, tone_index, climb, dura, e);
        }

        var res = [];
        for (var tone in profile_tones)
        {
            tone = profile_tones[tone];
            res.push(get_between(tone, climb));
        }

        draw_tone_prev($("#profile_dura"), res, climb);
    });
    

    
    $("#profile_editor").mouseup(function(e){
        profile_point_drag = null;
        drag_climb_orig = false;
        drag_val_orig = false;        
    });
    
    $("#profile_editor").on("wheel", function(e){
    if (profile_point_wheel != null)
        {
            var id_index = parseInt(profile_point_wheel.tone_index);
            var sub_index = parseInt(profile_point_wheel.sub_index);
            point_move_y(sub_index, id_index, -e.originalEvent.deltaY);
        }
    });  
    
    $(document).on("keydown", function(e){
    if (profile_point_wheel != null)
        {
            var diff_x = 0;            
            var diff_y = 0;
            
            if (e.originalEvent.key == "ArrowUp")
                diff_y = +1;
            if (e.originalEvent.key == "ArrowDown")
                diff_y = -1;
            if (e.originalEvent.key == "ArrowLeft")
                diff_x = -1;
            if (e.originalEvent.key == "ArrowRight")
                diff_x = +1;
                
        
            var id_index = parseInt(profile_point_wheel.tone_index);
            var sub_index = parseInt(profile_point_wheel.sub_index);
            
            if (diff_y != 0)
                point_move_y(sub_index, id_index, diff_y);

            if (diff_x != 0)
                point_move_x(sub_index, id_index, diff_x / 10.0);
        }
    });      
    

    $("#profile_freq").mousemove(function(e){
    
        var x = e.offsetX;
        var y = e.offsetY;
    
        var w = $("#profile_freq").width();
        var h = $("#profile_freq").height();
    
        var climb = x_to_climb(x, w);
        
        if (profile_point_drag != null)
        {
            var freq = y_to_freq(h - y, h);
            var tone_index = parseInt(profile_point_drag.tone_index);
            var sub_index = parseInt(profile_point_drag.sub_index);            
            
            if (sub_index == 0)
                climb = point_drag(sub_index, tone_index, climb, freq, e);
            
        }
        
        var res = [];
        for (var tone in profile_tones)
        {
            tone = profile_tones[tone];
            res.push(get_between(tone, climb));
        }

        draw_tone_prev($("#profile_dura"), res, climb);        

    });    
    
    $("#store").click(function (){
        var text = "";
        text += "tone_size\t" + profile_tones.length + "\n";
        for (var i in profile_tones)
        {
            var keys = sorted_list(Object.keys(profile_tones[i]));
            
            text += "tone_" + i + "_size\t" + keys.length + "\n";
            var j = 0;
            for (var k in keys)
            {
                var p = profile_tones[i][keys[k]];
                text += "tone_" + i + "_" + j + "_c\t" + Math.round(parseFloat(keys[k])*100) + "\n";
                text += "tone_" + i + "_" + j + "_f\t" + p[0] + "\n";
                text += "tone_" + i + "_" + j + "_d\t" + p[1] + "\n";
                j++;
            }
        }
        
        var blob = new Blob([text], {type: "text/plain"});
        saveAs(blob, "profile.cfg");
        console.log(text);
    });
});



