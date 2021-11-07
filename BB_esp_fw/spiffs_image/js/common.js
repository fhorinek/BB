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

