'use strict';

var CLIMB_STEPS = 10;
var CLIMB_DELTA = CLIMB_STEPS * 2;
var DURA_SCALE = 4000;

var F_MAX = 2000;

var profile_tones = [];


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
        if (prev_key >= climb - 0.1)
            climb = prev_key + 0.1;

    if (next_key != undefined)
        if (next_key <= climb + 0.1)
            climb = next_key - 0.1

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
        if (prev_key >= climb - 0.1)
            climb = prev_key + 0.1;

    if (next_key != undefined)
        if (next_key <= climb + 0.1)
            climb = next_key - 0.1

    delete profile_tones[profile_tone_selected][old_key];
    profile_tones[profile_tone_selected][climb] = [freq, dura];
    profile_tones[profile_tone_selected][climb][sub_index] = val;

    draw_line($("#profile_freq"), profile_tones[profile_tone_selected]);      
    draw_graph($("#profile_dura"), profile_tones);
    
    return climb;
}

//tone
//[climb] freq, duration

function load_profile_from_device(path)
{
    var data = {};
    data["path"] = path;
    
    close_modal();
    
    $.post({
        url: "api/get_file", 
        data: data,
        complete: function(res){
        	$("#profile-name").val(path.slice(path.lastIndexOf("/") + 1, path.indexOf(".cfg")));
            load_from_string(res.responseText);
        }
    });
}

function close_modal()
{
    document.getElementById("modal-control").checked = false;
}

function create_modal(title, text, link_list, cb)
{
    
    var body = document.createElement("p");
    
    if (text != "")
    {
        var p = document.createElement("p");
        p.append(text);
        body.append(p);
    }

    var ul = document.createElement("ul");
    for (var key in link_list)
    {
        var name = link_list[key][0];
        var link = link_list[key][1];
    
        var li = document.createElement("li"); 
        var a = document.createElement("a");
        a.textContent = name;
        a.href = "#";
        $(a).click(function(){
            console.log(link);
            cb(link);
        });
        li.append(a);
        
        ul.append(li);
    }

    body.append(ul);    
    document.getElementById("modal-title").textContent = title;
    
    var content = document.getElementById("modal-content");
    while (content.firstChild) 
        content.removeChild(content.lastChild);
    content.append(body);    
        
    document.getElementById("modal-control").checked = true;
}

function load_from_string(input)
{
    var text = input.split("\n");
    
    var data = {};
    
    for (var index in text)
    {
        var pair = text[index];
        pair = pair.split("\t");
        data[pair[0]] = pair[1];
    }
    
    var new_tones = [];
    for (var i = 0; i < data["tone_size"]; i++)
    {
	    new_tones[i] = [];
    
	    for (var j = 0; j < data["tone_" + i + "_size"]; j++)
	    {
	        var base = "tone_" + i + "_" + j + "_";
	        
	        var c = parseFloat(data[base + "c"]) / 100;
	        var d = parseInt(data[base + "d"]);
	        var f = parseInt(data[base + "f"]);
	        
	        new_tones[i][c] = [f, d];
	    }
    }
    
    profile_tones = new_tones;
    
    //clear points        
    $("[id^=poly_]").remove();
    $("[id^=tone_prev_]").remove();    
    $("[id^=poly_outline_]").remove();
    select_tone(0);
}

function save_to_string(tones)
{
    var text = "";
    text += "tone_size\t" + tones.length + "\n";
    for (var i in tones)
    {
        var keys = sorted_list(Object.keys(tones[i]));
        
        text += "tone_" + i + "_size\t" + keys.length + "\n";
        var j = 0;
        for (var k in keys)
        {
            var p = tones[i][keys[k]];
            text += "tone_" + i + "_" + j + "_c\t" + Math.round(parseFloat(keys[k])*100) + "\n";
            text += "tone_" + i + "_" + j + "_f\t" + p[0] + "\n";
            text += "tone_" + i + "_" + j + "_d\t" + p[1] + "\n";
            j++;
        }
    }
    return text;
}

$(function() {

    var dura = $("#profile_dura");
    var freq = $("#profile_freq");
    
    draw_axis_dura(dura);
    draw_legend(dura);    

    draw_axis_freq(freq);

    
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
    
    $("#save_file").click(function ()
    {
        var text = save_to_string(profile_tones);
    
        
        var blob = new Blob([text], {type: "text/plain"});
        saveAs(blob, "profile.cfg");
        //console.log(text);
    });
    
    $("#load_file").click(function()
    {
    	$("#file-selector").click();
    });
    
    $("#prev_sound").change(function(){
        if (!$('#prev_sound').prop('checked'))
        {
            var post_data = {};
            post_data["cnt"] = 0;
            send_ajax("api/sound", post_data);
        }
    });
    
    $("#file-selector").change(function(files){
    	var file = this.files[0];
    	
    	if (this.files.length != 1 || (file.name.slice(-4) != ".cfg"))
    	{
    		alert("Please select .cfg file");
    		this.val(null);
    		return;
    	}
    	
    	$("#profile-name").val(file.name.slice(0, file.name.indexOf(".cfg")));
    	
    	var reader = new FileReader();
    	reader.onload = function(e) 
    	{
    	    var text = String.fromCharCode.apply(null, new Uint8Array(e.target.result));
            load_from_string(text);
    	};
    	  
    	reader.readAsArrayBuffer(file);
    });
    
    $("#load_device").click(function()
    {
        var data = {};
        var path = "config/vario";
        data["path"] = path;
        data["filter"] = 1;
        
         $.post({
            url: "api/list_fs", 
            data: data,
            complete: function(res){
                var data = JSON.parse(res.responseText);
                
                var link_list = [];
                for (var key in data)
                {
                    var name = data[key][0];
                    var label = name.slice(0, name.indexOf(".cfg"));
                    link_list.push([label, path + "/" + name]);
                }
                
                create_modal("Select profile", "select vario profile from strato", link_list, load_profile_from_device);
            }
        });
    });
    
    $("#save_device").click(function()
    {
        if (!confirm("Save this profile to strato as '" + $("#profile-name").val() + "'?"))
            return;
        
    
        var data = {};
        data["path"] = "config/vario/" + $("#profile-name").val() + ".cfg";
        data["data"] = save_to_string(profile_tones);
        
         $.post({
            url: "api/save_file", 
            data: data,
/*            complete: function(res){
                alert(res.responseText);
            }*/
        });
    });    
    
    $("#load_default").click(function()
    {
        if (!confirm("Load default system profile"))
            return;

        load_profile_from_device("system/assets/defaults/vario/default.cfg");
        $("#profile-name").val("default");
    });       
    
    load_profile_from_device("config/vario/default.cfg");
    $("#profile-name").val("default");
});



