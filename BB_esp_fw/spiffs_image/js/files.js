'use strict';

var current_dir = "";

function download_file(path)
{
    var data = {};
    data["path"] = path;
    
    $.post({
        url: "api/get_file", 
        data: data,
        complete: function(res){
        	var name = path.slice(path.lastIndexOf("/") + 1);
            var blob = new Blob([res.responseText], {type: "application/octet-stream"});
            saveAs(blob, name);
        }
    });
}

function create_listing(data, path)
{
    function cmp(a, b)
    {
        if (a[1] != b [1])
            return a[1] > b[1] ? -1 : 1;
        
        return a[0].localeCompare(b[0]) ? 1 : -1;
    }

    current_dir = path;

    var body = document.createElement("p");

    var ul = document.createElement("ul");
    
    data.sort(cmp);
    
    for (var key in data)
    {
        var name = data[key][0];
        var type = data[key][1];
    
        var folder = document.createElement("img"); 
        folder.src = "img/folder.svg";
        folder.classList.add("icon");
    
        var file = document.createElement("img"); 
        file.src = "img/file.svg";
        file.classList.add("icon");    
        
        var back = document.createElement("img"); 
        back.src = "img/arrow-left.svg";
        back.classList.add("icon"); 
    
        var li = document.createElement("li"); 
        var a = document.createElement("a");
        a.textContent = name;
        a.href = "#";
        
        if (type == 2)
        {
            a.link = path + "/" + name;
            $(a).click(function(){
                list_dir(this.link);
            });
            li.append(folder);
        }
        else if (type == 1)
        {
            a.link = path + "/" + name;
            $(a).click(function(){
                download_file(this.link);
            });        
            li.append(file);            
        } else if (type == 10)
        {
            a.link = path.slice(0, path.lastIndexOf("/"));
            $(a).click(function(){
                list_dir(this.link);
            });
            li.append(back);
        }
        
        li.append(a);
        
        ul.append(li);
    }

    body.append(ul);    
    document.getElementById("files-path").textContent = "[Strato] " + path;
    
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
                data = [["back", 10]].concat(data);
            }
            create_listing(data, path);
        }
    });

}

$(function() {    
    $("#refresh_dir").click(function()
    {
        list_dir(current_dir);
    });
    
    $("#upload_file").click(function()
    {
        $("#file-selector").click();
    });
    
    $("#file-selector").change(function(files){
    	var file = this.files[0];
    	var reader = new FileReader();
    	
    	reader.onload = function(e) 
    	{
            var data = {};

            data["path"] = current_dir + "/" + file.name;
            data["data"] = e.target.result;
            
             $.post({
                url: "api/save_file", 
                data: data,
                complete: function(res){
//                    alert(res.responseText);
                    list_dir(current_dir);
                }
            });
    	};
    	  
    	reader.readAsArrayBuffer(file);
    });    
    
    list_dir("/logs");
});
