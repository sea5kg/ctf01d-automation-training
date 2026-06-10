(function () {
  "use strict";

  if (!window.fhq) window.fhq = {};
  if (!window.fhq.ui) window.fhq.ui = {};

  var api = {
    flag: "api/v1/flag/",
    signup: "api/v1/signup/",
    rating: "api/v1/rating/"
  };

  var ratingTimer = undefined;
  var defaultFlag = "c01d4567-e89b-12d3-a456-426600000010";
  var ratingLabels = ["Звание", "Баллы", "Имя", "Атака", "Штраф", "Попытки"];

  window.fhq.ui.ratingName = [
    { s: 0, img: "01-ryadovoi.png", title: "Рядовой" },
    { s: 25, img: "02-efreitor.png", title: "Ефрейтор" },
    { s: 50, img: "03-ml_serjant.png", title: "Младший сержант" },
    { s: 100, img: "04_serjant.png", title: "Сержант" },
    { s: 150, img: "05-st_serjant.png", title: "Старший сержант" },
    { s: 300, img: "06-starshina.png", title: "Старшина" },
    { s: 400, img: "07-praporschik.png", title: "Прапорщик" },
    { s: 500, img: "08-st_praporschik.png", title: "Старший прапорщик" },
    { s: 1000, img: "09-ml_leitinant.png", title: "Младший лейтинант" },
    { s: 1500, img: "10-leitinant.png", title: "Лейтинант" },
    { s: 2000, img: "11-st_leitinant.png", title: "Старший лейтинант" },
    { s: 4000, img: "12-kapitan.png", title: "Капитан" },
    { s: 5000, img: "13-major.png", title: "Майор" },
    { s: 6000, img: "14-podpolkovnik.png", title: "Подполковник" },
    { s: 8000, img: "15-polkovnik.png", title: "Полковник" },
    { s: 10000, img: "16-general_major.png", title: "Генерал-майор" },
    { s: 15000, img: "17-general_leitinant.png", title: "Генерал-лейтинант" },
    { s: 20000, img: "18-general_polkovnik.png", title: "Генерал-полковник" },
    { s: 25000, img: "19-general_armii.png", title: "Генерал Армии" },
    { s: 50000, img: "20-marshal.png", title: "Маршал" },
    { s: 100000, img: "21-generalissimus.png", title: "Генералиссимус" }
  ];

  function qs(selector, scope) {
    return (scope || document).querySelector(selector);
  }

  function buildQuery(params) {
    return new URLSearchParams(params).toString();
  }

  function setStatus(element, message, state) {
    if (!element) return;
    element.textContent = message || "";
    element.classList.remove("is-loading", "is-success", "is-warning", "is-error");
    if (state) {
      element.classList.add("is-" + state);
    }
  }

  function setButtonLoading(button, isLoading) {
    if (!button) return;
    button.disabled = isLoading;
    button.setAttribute("aria-busy", String(isLoading));
    button.classList.toggle("is-loading", isLoading);
  }

  function formatRegistrationError(xhrLike) {
    if (xhrLike && xhrLike.status === 404) {
      return "Ненайдена страница api/v1/signup/";
    }

    if (xhrLike && xhrLike.error && xhrLike.error.code) {
      return "Ошибка " + xhrLike.error.code + " при регистрации: " + xhrLike.error.message_ru;
    }

    return "Ошибка 0 при регистрации: ";
  }

  function formatFlagError(xhrLike) {
    if (xhrLike && xhrLike.error && xhrLike.error.code) {
      return "Ошибка " + xhrLike.error.code + " при отправке флага: " + xhrLike.error.message_ru;
    }

    return "Ошибка 0 при отправке флага: ";
  }

  function classifyFlagResponse(text) {
    if (text.indexOf("[FLAG ACCEPTED]") !== -1) return "success";
    if (text.indexOf("[FLAG WAS NOT ACCEPTED]") !== -1) return "warning";
    return "success";
  }

  function requestText(url, params) {
    return fetch(url + "?" + buildQuery(params), {
      method: "GET",
      headers: { Accept: "text/plain, */*" }
    }).then(function (response) {
      return response.text().then(function (text) {
        if (!response.ok) {
          var payload = { status: response.status, text: text };
          try {
            Object.assign(payload, JSON.parse(text));
          } catch (error) {
            payload.parseError = error;
          }
          throw payload;
        }
        return text;
      });
    });
  }

  function requestJson(url, params) {
    return fetch(url + "?" + buildQuery(params), {
      method: "GET",
      headers: { Accept: "application/json" }
    }).then(function (response) {
      return response.json().catch(function () {
        return {};
      }).then(function (json) {
        if (!response.ok) {
          throw Object.assign({ status: response.status }, json);
        }
        return json;
      });
    });
  }

  function getRank(score) {
    var ranks = window.fhq.ui.ratingName;
    var numericScore = Number(score);

    if (numericScore < 0 || Number.isNaN(numericScore)) {
      return null;
    }

    for (var i = 0; i < ranks.length; i += 1) {
      var current = ranks[i];
      var next = ranks[i + 1];
      var max = next ? next.s : 100000000;

      if (numericScore >= current.s && numericScore < max) {
        return current;
      }
    }

    return null;
  }

  function makeCell(label, value) {
    var cell = document.createElement("td");
    cell.setAttribute("data-label", label);

    if (value instanceof Node) {
      cell.appendChild(value);
    } else {
      cell.textContent = value;
    }

    return cell;
  }

  function makeRankCell(score) {
    var rank = getRank(score);

    if (!rank) {
      return "-";
    }

    var img = document.createElement("img");
    img.src = "images/" + rank.img;
    img.title = rank.title;
    img.alt = rank.title;
    return img;
  }

  function renderRating(items) {
    var tbody = qs("#rating tbody");
    if (!tbody) return;

    var fragment = document.createDocumentFragment();

    items.forEach(function (item, index) {
      var row = document.createElement("tr");
      row.className = "rating-row";
      row.id = "rating" + index;
      row.appendChild(makeCell(ratingLabels[0], makeRankCell(item.score)));
      row.appendChild(makeCell(ratingLabels[1], item.score));
      row.appendChild(makeCell(ratingLabels[2], item.name));
      row.appendChild(makeCell(ratingLabels[3], item.attack));
      row.appendChild(makeCell(ratingLabels[4], item.penalty));
      row.appendChild(makeCell(ratingLabels[5], item.tries));
      fragment.appendChild(row);
    });

    tbody.replaceChildren(fragment);
  }

  window.fhq.ui.accept_flag = function () {
    var result = qs("#accept-result");
    var button = qs("#accept_flag");
    var data = {
      token: qs("#accept-token").value,
      flag: qs("#accept-flag").value
    };

    setButtonLoading(button, true);
    setStatus(result, "Отправка...", "loading");

    requestText(api.flag, data)
      .then(function (response) {
        setStatus(result, response, classifyFlagResponse(response));
        window.fhq.ui.updateLink();
      })
      .catch(function (error) {
        setStatus(result, formatFlagError(error), "error");
      })
      .finally(function () {
        setButtonLoading(button, false);
      });
  };

  window.fhq.ui.get_token = function () {
    var result = qs("#user-token");
    var button = qs("#get_token");
    var data = {
      username: qs("#user-name").value
    };

    setButtonLoading(button, true);
    setStatus(result, "Получение токена...", "loading");

    requestJson(api.signup, data)
      .then(function (response) {
        var token = response.result.secret_token;
        setStatus(result, token, "success");
        qs("#accept-token").value = token;
        localStorage.setItem("secret_token", token);
        window.fhq.ui.updateLink();
      })
      .catch(function (error) {
        setStatus(result, formatRegistrationError(error), "error");
      })
      .finally(function () {
        setButtonLoading(button, false);
      });
  };

  window.ctf01dUpdateRating = function () {
    if (!ratingTimer) {
      ratingTimer = setInterval(window.ctf01dUpdateRating, 5000);
    }

    requestJson(api.rating, {})
      .then(function (response) {
        renderRating(response.result || []);
      })
      .catch(function () {
        renderRating([]);
      });
  };

  window.fhq.ui.updateLink = function () {
    var tokenInput = qs("#accept-token");
    var flagInput = qs("#accept-flag");
    var link = qs("#link_send");
    var savedToken = localStorage.getItem("secret_token");
    var token = tokenInput && tokenInput.value ? tokenInput.value : savedToken || "your_token";
    var flag = flagInput && flagInput.value ? flagInput.value : defaultFlag;
    var params = buildQuery({ token: token, flag: flag });
    var basePath = location.pathname.endsWith("/") ? location.pathname : location.pathname.replace(/[^/]*$/, "");
    var href = location.origin + basePath + api.flag + "?" + params;

    if (!link) return;
    link.href = href;
    link.textContent = api.flag + "?" + params;
  };

  document.addEventListener("click", function (event) {
    var tokenButton = event.target.closest("#get_token");
    var flagButton = event.target.closest("#accept_flag");

    if (tokenButton) {
      window.fhq.ui.get_token();
    }

    if (flagButton) {
      window.fhq.ui.accept_flag();
    }
  });

  document.addEventListener("input", function (event) {
    if (event.target.matches("#accept-token, #accept-flag")) {
      window.fhq.ui.updateLink();
    }
  });

  document.addEventListener("keydown", function (event) {
    if (event.key !== "Enter") return;

    if (event.target.matches("#user-name")) {
      event.preventDefault();
      window.fhq.ui.get_token();
    }

    if (event.target.matches("#accept-token, #accept-flag")) {
      event.preventDefault();
      window.fhq.ui.accept_flag();
    }
  });

  document.addEventListener("DOMContentLoaded", function () {
    var secretToken = localStorage.getItem("secret_token");

    if (secretToken !== null) {
      setStatus(qs("#user-token"), secretToken, "success");
      qs("#accept-token").value = secretToken;
    }

    window.fhq.ui.updateLink();
    window.ctf01dUpdateRating();
  });
})();
