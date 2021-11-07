
var pos = false;
var target = false;

var mymap = L.map('mapid');

var pg_icon = L.icon({
    iconUrl: 'img/pg_icon.png',
    iconSize: [50, 21],
    iconAnchor: [25, 10],
    popupAnchor: [0, 0],
    className: "map_pg_icon"
});

function fake_gnss(lat, lng, spd, hdg)
{
    var GNSS_MUL = 10000000;

    var data = {};
    data["lat"] = Math.round(lat * GNSS_MUL);
    data["lon"] = Math.round(lng * GNSS_MUL);
    data["speed"] = Math.round(spd);
    data["heading"] = Math.round(hdg);
    data["fix"] = 3;
    data["time"] = Math.round(Date.now() / 1000);

    send_ajax("api/fake_gnss", data);
}

function rotate_icon(deg)
{
    var x = parseInt($(".map_pg_icon").css('transform').split(",")[4]);
    var y = parseInt($(".map_pg_icon").css('transform').split(",")[5]);
    
    $(".map_pg_icon").css('transform', "translate(" + x + "px, " + y + "px) rotate(" + deg + "deg)")
}

function onMapClick(e) 
{
    if (target == false)
    {
        pos = L.marker(e.latlng, {icon: pg_icon}).addTo(mymap);
        target = L.marker(e.latlng).addTo(mymap);
    }
    else
    {
        target.setLatLng(e.latlng);
    }
}


$(function()
{

    L.tileLayer('https://api.mapbox.com/styles/v1/{id}/tiles/{z}/{x}/{y}?access_token={accessToken}', {
        attribution: 'Map data &copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors, Imagery © <a href="https://www.mapbox.com/">Mapbox</a>',
        maxZoom: 18,
        id: 'mapbox/streets-v11',
        tileSize: 512,
        zoomOffset: -1,
        accessToken: 'pk.eyJ1Ijoic2t5YmVhbjAxIiwiYSI6ImNrczBkcWl4YTExZmYzMm85dW1uMDIwZngifQ.H0LHqMBExbGZZ1932dlXnQ'
    }).addTo(mymap);

    mymap.locate({setView: true, maxZoom: 16});
    mymap.on('click', onMapClick);


    var period = 100;

    setInterval(function()
    {
        if (pos && target)
        {
            var dist = pos.getLatLng().distanceTo(target.getLatLng());
            
            if (dist > 0)
            {
                var lat_d = target.getLatLng().lat - pos.getLatLng().lat;
                var lng_d = target.getLatLng().lng - pos.getLatLng().lng;
                var lat = pos.getLatLng().lat;
                var lng = pos.getLatLng().lng;

                var speed = $("#sim_spd").val();
            
                var step = speed * (period / 1000);
                var m = step / dist;

                var hdg = Math.atan2(lng_d, lat_d) * 180 / Math.PI;
                if (hdg < 0)
                    hdg += 360
                    
                
                if (dist < step)
                {
                    pos.setLatLng(target.getLatLng());
                    speed = 0;
                }
                else
                {
                    pos.setLatLng([lat + lat_d * m, lng + lng_d * m]);
                }
                    
                rotate_icon(hdg);  
                
                lat = pos.getLatLng().lat;
                lng = pos.getLatLng().lng;
                
                fake_gnss(lat, lng, speed, hdg);
            }
        
        }
    }, period);



});
