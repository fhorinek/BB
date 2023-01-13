'use strict';

var current_dir = "";

function download_file(path)
{
    var data = {};
    data["path"] = path;
    
    $.post({
        url: "api/get_file", 
        data: data,
        dataType: 'binary'
    }).then(function(res)
        {
        	var name = path.slice(path.lastIndexOf("/") + 1);
            var blob = new Blob([res], {type: "application/octet-stream"});
            saveAs(blob, name);
        });
}

function delete_file(path)
{
    var data = {};
    data["path"] = path;
    
    $.post({
        url: "api/del_file", 
        data: data,
        complete: function(res)
        {
            list_dir(current_dir);
        }
    });
}


function create_listing(data, path)
{
    function cmp(a, b)
    {
        if (a[2] != b [2])
            return a[2] > b[2] ? -1 : 1;
        
        return a[0].localeCompare(b[0]) ? 1 : -1;
    }

    current_dir = path;

    var body = document.createElement("p");

    var table = document.createElement("table");
    var tbody = document.createElement("tbody");

    var tr = document.createElement("tr"); 
    tbody.append(tr);
    
    data.sort(cmp);
    
    for (var key in data)
    {
        var name = data[key][0];
        var size = data[key][1];
        var type = data[key][2];
    
        var folder = document.createElement("img"); 
        folder.src = "img/folder.svg";
        folder.classList.add("icon");
    
        var file = document.createElement("img"); 
        file.src = "img/file-text.svg";
        file.classList.add("icon");    
        
        var back = document.createElement("img"); 
        back.src = "img/arrow-left.svg";
        back.classList.add("icon"); 
    
        var tr = document.createElement("tr"); 
        var a = document.createElement("a");
        a.textContent = name;
        a.href = "#";

        var trash = document.createElement("img"); 
        trash.src = "img/trash-2.svg";
        trash.classList.add("icon");  
        
        var a_del = document.createElement("a");
        a_del.append(trash);
        a_del.href = "#";

        var td;
        
        if (type == 2)
        {
            a.link = path + "/" + name;
            $(a).click(function(){
                list_dir(this.link);
            });

            td = document.createElement("td");
            td.append(folder);
            tr.append(td);

            td = document.createElement("td");
            td.append(a);
            td.classList.add("name");
            tr.append(td);

            td = document.createElement("td");
            td.classList.add("size");
            tr.append(td);    

            td = document.createElement("td");
            tr.append(td);            
        }
        else if (type == 1)
        {
            a.link = path + "/" + name;
            $(a).click(function(){
                download_file(this.link);
            });        

            a_del.link = path + "/" + name;
            $(a_del).click(function(){
                delete_file(this.link);
            });        

            td = document.createElement("td");
            td.append(file);
            tr.append(td);
            
            td = document.createElement("td");
            td.append(a);
            td.classList.add("name");
            tr.append(td);

            td = document.createElement("td");
            td.append(size);
            td.classList.add("size");
            tr.append(td);            
            
            td = document.createElement("td");
            td.append(a_del);
            tr.append(td);
            
        }
        else if (type == 10)
        {
            a.link = path.slice(0, path.lastIndexOf("/"));
            $(a).click(function(){
                list_dir(this.link);
            });
            
            td = document.createElement("td");
            td.append(back);
            tr.append(td);
            
            td = document.createElement("td");
            td.append(a);
            tr.append(td);

            td = document.createElement("td");
            tr.append(td);

            td = document.createElement("td");
            tr.append(td);
        }
        
        
        tbody.append(tr);
    }

    table.append(tbody);
    body.append(table);    
    document.getElementById("files-path").textContent = "Strato" + path;
    
    var content = document.getElementById("files-content");
    while (content.firstChild) 
        content.removeChild(content.lastChild);
        
    content.append(body);    
    document.getElementById("files-content").checked = true;
}

function list_dir(path)
{
    var data = {};
    data["path"] = path;
    data["filter"] = 3;
    
     $.post({
        url: "api/list_fs", 
        data: data,
        complete: function(res){
            var data = JSON.parse(res.responseText);

            if (path.length > 0)
            {
                data = [["back", 0, 10]].concat(data);
            }
            create_listing(data, path);
        }
    });

}

var modal_progress = null;
var modal_cancel = false;


function close_modal()
{
    document.getElementById("modal-control").checked = false;
    modal_progress = null;
}



function create_modal(title, text, progress = true)
{
    
    var body = document.createElement("p");
    
    if (text != "")
    {
        var p = document.createElement("p");
        p.append(text);
        body.append(p);
    }

    if (progress)
    {
        modal_progress = document.createElement("progress");
        body.append(modal_progress);

        var but = document.createElement("input");
        but.type = "button";
        but.value = "Cancel";

        modal_cancel = false;
        $(but).click(function() {
            modal_cancel = true;
        });

        body.append(but);
    }
 
    document.getElementById("modal-title").textContent = title;
    
    var content = document.getElementById("modal-content");
    while (content.firstChild) 
        content.removeChild(content.lastChild);
    content.append(body);    
        
    document.getElementById("modal-control").checked = true;
}

const CHUNK = 15 * 1024;

function send_file_cb(data, res)
{
    var r = JSON.parse(res.responseText);

    if (modal_cancel)
    {
        close_modal();
        return;
    }
    
    if (r.done)            
    {
        list_dir(current_dir);
        close_modal();
    }   
    else
    {
        var data_packet = {}; 
        data_packet["file_id"] = r.file_id;
        data_packet["index"] = r.index;
        data_packet["size"] = data.length;        
        data_packet["data64"] = btoa(data.slice(r.index, r.index + CHUNK));

        modal_progress.value = r.index / data.length;
        
        $.post({
            url: "api/save_file", 
            data: data_packet,
            complete: function(res){
                send_file_cb(data, res);
            }
        });        
    }
}


function send_file(path, data)
{
    var data_packet = {}; 
    data_packet["path"] = path;
    data_packet["size"] = data.length;
    data_packet["data64"] = btoa(data.slice(0, CHUNK));

    create_modal("Uploading...", path);
    
    $.post({
        url: "api/save_file", 
        data: data_packet,
        complete: function(res){
            send_file_cb(data, res);
        }
    });


}

$(function() {    
     $.ajaxSetup({
        beforeSend: function (jqXHR, settings) {
          if (settings.dataType === 'binary')
            settings.xhr = () => $.extend(new window.XMLHttpRequest(), {responseType:'arraybuffer'})
        }
      })
    
    $("#refresh_dir").click(function()
    {
        list_dir(current_dir);
    });
    
    $("#upload_file").click(function()
    {
        $("#file-selector").click();
    });
    
    $("#file-selector").change(function(files){
        if (this.files.length == 1)
        {
        	var file = this.files[0];
        	var reader = new FileReader();
        	
        	reader.onload = function(e) 
        	{
                var path = current_dir + "/" + file.name;
                var data = e.target.result;
                
                send_file(path, data);
        	};
        	  
        	reader.readAsBinaryString(file);
        }
        $("#file-selector").val("");
    });    

    list_dir("/logs");
});
