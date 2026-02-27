/* MIT License

* Copyright (c) 2015-2026 Evgenii Sopov

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

// https://github.com/sea5kg/gtree

#pragma once

#include <string>

namespace gtree {

class ErrorInfo {
public:
  ErrorInfo(
    int code,
    const std::string &msg,
    const std::string &msg_ru,
    const std::string &empty
  ) : code(code), msg(msg), msg_ru(msg_ru), empty(empty) {};
  ErrorInfo replace(std::string var_name, std::string var_value) const {
    if (var_name.empty()) {
        return ErrorInfo(code, msg, msg_ru, empty);
    }

    std::string copy_msg = msg;
    size_t start_pos = 0;
    while ((start_pos = copy_msg.find(var_name, start_pos)) != std::string::npos) {
        copy_msg.replace(start_pos, var_name.length(), var_value);
        start_pos += var_value.length(); // In case 'to' contains 'sFrom', like replacing 'x' with 'yx'
    }

    std::string copy_msg_ru = msg_ru;
    start_pos = 0;
    while ((start_pos = copy_msg_ru.find(var_name, start_pos)) != std::string::npos) {
        copy_msg_ru.replace(start_pos, var_name.length(), var_value);
        start_pos += var_value.length(); // In case 'to' contains 'sFrom', like replacing 'x' with 'yx'
    }
    return ErrorInfo(code, copy_msg, copy_msg_ru, empty);
  };
  const int code;
  const std::string msg;
  const std::string msg_ru;
  const std::string empty;
};

} // namespace gtree

static const gtree::ErrorInfo ERR_01001_ONLY_POST_OR_GET_REQUESTS(
    1001,
    "Only 'POST' or 'GET' requests will be handled.",
    "Обрабатываться будут только запросы типа 'POST' или 'GET'.",
    ""
);

static const gtree::ErrorInfo ERR_01002_INVALID_INCOMING_JSON(
    1002,
    "Invalid incoming json",
    "Недопустимый входящий JSON.",
    ""
);

static const gtree::ErrorInfo ERR_01003_EXPECTED_JSON_INPUT(
    1003,
    "Expected json object input.",
    "Ожидаемый ввод: объект JSON.",
    ""
);

static const gtree::ErrorInfo ERR_01004_MISSING_FIELD_JSONRPC(
    1004,
    "Missing field 'jsonrpc'",
    "Отсутствует поле 'jsonrpc'.",
    ""
);

static const gtree::ErrorInfo ERR_01005_MISSING_FIELD_METHOD(
    1005,
    "Missing field 'method'",
    "Отсутствует поле 'method'.",
    ""
);

static const gtree::ErrorInfo ERR_01006_UNKNOWN_METHOD(
    1006,
    "Unknown method.",
    "Неизвестный метод.",
    ""
);

static const gtree::ErrorInfo ERR_01007_MISSING_OR_WRONG_FIELD_PARAMS(
    1007,
    "Missing or unexpected type for field 'params'.",
    "Отсутствует или указан неожиданный тип для поля 'params'.",
    ""
);

static const gtree::ErrorInfo ERR_10015_MISSING_FIELD_USERNAME(
    10015,
    "Missing field 'username' or wrong type.",
    "Отсутствует поле 'username' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10016_MISSING_FIELD_FLAG(
    10016,
    "Missing field 'flag' or wrong type.",
    "Отсутствует поле 'flag' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10017_MISSING_FIELD_TOKEN(
    10017,
    "Missing field 'token' or wrong type.",
    "Отсутствует поле 'token' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10018_USER_ALREADY_EXISTS(
    10018,
    "User already exists '$username$'",
    "Пользователь уже существует '$username$'",
    ""
);

static const gtree::ErrorInfo ERR_10019_COULD_NOT_CREATE_USER(
    10019,
    "Could not create user '$username$'",
    "Не удалось создать пользователя '$username$'",
    ""
);

static const gtree::ErrorInfo ERR_10025_USERNAME_BY_TOKEN_NOT_FOUND(
    10025,
    "User not found by token '$token$'.",
    "Пользователь по токену '$token$' не найден.",
    ""
);

static const gtree::ErrorInfo ERR_10030_USERNAME_TOO_SHORT(
    10030,
    "Username '$username$' too short",
    "Имя '$username$' слишком короткое",
    ""
);

static const gtree::ErrorInfo ERR_10031_USERNAME_TOO_LONG(
    10031,
    "Username '$username$' too long",
    "Имя '$username$' слишком длинное",
    ""
);
