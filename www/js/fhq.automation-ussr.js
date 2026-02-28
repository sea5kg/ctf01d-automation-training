if(!window.fhq) window.fhq = {};
if(!window.fhq.ui) window.fhq.ui = {};


window.fhq.ui.accept_flag = function(){
  $("#accept-result").html("Отправка...");
  var data = {};
  data.token = $("#accept-token").val();
  data.flag = $("#accept-flag").val();
  $.ajax({
    url: 'api/v1/flag/',
    type: 'GET',
    data: data,
    success: function(response){
      $("#accept-result").html(response);
    },
    error: function(xhr){
      $("#accept-result").html("Ошибка " + xhr.responseJSON.error.code + " при регистрации: " + xhr.responseJSON.error.message_ru)
    }
  });
}

window.fhq.ui.get_token = function(){
  $("#user-token").html("Получение токена...");
  var data = {};
  data.username = $("#user-name").val();
  $.ajax({
    url: 'api/v1/signup/',
    type: 'GET',
    data: data,
    success: function(response){
      $("#user-token").html(response["result"]["secret_token"]);
      $("#accept-token").val(response["result"]["secret_token"]);
      localStorage.setItem("secret_token", response["result"]["secret_token"]);
      window.fhq.ui.updateLink();
    },
    error: function(xhr, data, data1){
      // console.log(xhr, data, data1)
      if (xhr.status == 404) {
        $("#user-token").html("Ненайдена страница api/v1/signup/");
      } else {
        $("#user-token").html(
          "Ошибка " + xhr.responseJSON.error.code + " при регистрации: " + xhr.responseJSON.error.message_ru
        );
      }
    }
  });
}

var ur = undefined;

window.fhq.ui.ratingName = [
  { s: 0,      img: '01-ryadovoi.png',       title: 'Рядовой' },
  { s: 25,    img: '02-efreitor.png',       title: 'Ефрейтор' },
  { s: 50,    img: '03-ml_serjant.png',       title: 'Младший сержант' },
  { s: 100,    img: '04_serjant.png',        title: 'Сержант' },
  { s: 150,    img: '05-st_serjant.png',       title: 'Старший сержант' },
  { s: 300,    img: '06-starshina.png',       title: 'Старшина' },
  { s: 400,    img: '07-praporschik.png',       title: 'Прапорщик' },
  { s: 500,    img: '08-st_praporschik.png',     title: 'Старший прапорщик' },
  { s: 1000,     img: '09-ml_leitinant.png',     title: 'Младший лейтинант' },
  { s: 1500,     img: '10-leitinant.png',       title: 'Лейтинант' },
  { s: 2000,     img: '11-st_leitinant.png',     title: 'Старший лейтинант' },
  { s: 4000,     img: '12-kapitan.png',         title: 'Капитан' },
  { s: 5000,     img: '13-major.png',         title: 'Майор' },
  { s: 6000,     img: '14-podpolkovnik.png',     title: 'Подполковник' },
  { s: 8000,    img: '15-polkovnik.png',       title: 'Полковник' },
  { s: 10000,    img: '16-general_major.png',     title: 'Генерал-майор' },
  { s: 15000,    img: '17-general_leitinant.png',   title: 'Генерал-лейтинант' },
  { s: 20000,    img: '18-general_polkovnik.png',   title: 'Генерал-полковник' },
  { s: 25000,    img: '19-general_armii.png',     title: 'Генерал Армии' },
  { s: 50000,    img: '20-marshal.png',         title: 'Маршал' },
  { s: 100000,    img: '21-generalissimus.png',     title: 'Генералиссимус' },
];

window.ctf01dUpdateRating = function(){
  // console.log("update rating");
  if(!ur) ur = setInterval(ctf01dUpdateRating, 5000);
  $.ajax({
    url: 'api/v1/rating/',
    type: 'GET',
    success: function(resp){
      // console.log(resp);
      for(var i = 0; i < resp.result.length; i++){
        var t = resp.result[i];
        if(($("#rating" + i)).length == 0){
          $("#rating").append('<div class="rating row" id="rating' + i + '"></div>');
        }

        var diff = fhq.ui.ratingName;

        var pogon = '-';
        if(t.score >= 0){
          for (var i1 = 0; i1 < diff.length; i1++){
            var d = diff[i1];
            var s_min = d.s;
            var s_max = diff[i1+1] ? diff[i1+1].s : 100000000;
            if(t.score >= s_min && t.score < s_max){
              pogon = '<img src="images/' + d.img + '" title="' + d.title + '"/>'
            }
          }
        }

        $("#rating" + i).html('<div class="cell">' + pogon + '</div>'
          + '<div class="cell">' + t.score + '</div>'
          + '<div class="cell">' + t.name + '</div>'
          + '<div class="cell">' + t.attack + '</div>'
          + '<div class="cell">' + t.shtraf + '</div>'
          + '<div class="cell">' + t.tries + '</div>'
        );
      }
    }
  });
}

window.fhq.ui.updateLink = function() {
  var secret_token = localStorage.getItem("secret_token");
  if (secret_token === null) {
    secret_token = "your_token";
  }
  var flag = "c01d4567-e89b-12d3-a456-426600000010"
  var link = "https://" + location.host + location.pathname + "api/v1/flag/?token=" + secret_token + "&flag=" + flag;
  $('#link_send').attr({'href': link});
  $('#link_send').html(link);
}

$(document).ready(function(){
  $('#accept_flag').unbind().bind('click', function(){
    fhq.ui.accept_flag();
  });

  $('#get_token').unbind().bind('click', function(){
    fhq.ui.get_token();
  });

  var secret_token = localStorage.getItem("secret_token");
  if (secret_token !== null) {
    $("#user-token").html(secret_token);
    $("#accept-token").val(secret_token);
  }

  window.fhq.ui.updateLink();

  ctf01dUpdateRating();
})
