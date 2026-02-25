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

static const gtree::ErrorInfo ERR_01008_YOU_ALREADY_AUTHORIZED(
    1008,
    "You already authorized.",
    "Вы уже авторизованы..",
    ""
);

static const gtree::ErrorInfo ERR_01009_NOT_AUTHORIZED(
    1009,
    "You not authorized.",
    "Вы не авторизованы.",
    ""
);

static const gtree::ErrorInfo ERR_01010_ALLOWED_ONLY_FOR_ADMIN(
    1010,
    "Allowed only for admin.",
    "Доступно только для администраторов.",
    ""
);


// static const gtree::ErrorInfo ERR_02001_ALLOWED_ONLY_FOR_ADMIN(
//     2001,
//     "Allowed only for admin.",
//     "Доступно только для администраторов.",
//     ""
// );

static const gtree::ErrorInfo ERR_10004_MISSING_FIELD_EMAIL(
    10004,
    "Missing field 'email' or wrong type.",
    "Отсутствует поле 'email' или указан неверный тип..",
    ""
);

static const gtree::ErrorInfo ERR_10008_YOU_CAN_NOT_DELETE_YOURSELF(
    10008,
    "You can not delete yourself.",
    "Вы не можете удалить себя.",
    ""
);

static const gtree::ErrorInfo ERR_10009_USER_NOT_FOUND_WITH_EMAIL(
    10009,
    "User not found with email '$email$'.",
    "Пользователь с адресом электронной почты '$email$' не найден.",
    ""
);

static const gtree::ErrorInfo ERR_10010_COULD_NOT_REMOVE_USER_WITH_EMAIL(
    10010,
    "Could not remove user '$email$'.",
    "Не удалось удалить пользователя '$email$'.",
    ""
);

static const gtree::ErrorInfo ERR_10011_COULD_NOT_DID_LOGOUT(
    10011,
    "Could not did logout.",
    "Не удалось выйти из системы.",
    ""
);

static const gtree::ErrorInfo ERR_10012_MISSING_FIELD_EMAIL(
    10012,
    "Missing field 'email' or wrong type.",
    "Отсутствует поле 'email' или указан неверный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10013_MISSING_FIELD_NEW_PASS(
    10013,
    "Missing field 'new_pass' or wrong type.",
    "Отсутствует поле 'new_pass' или указан неверный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10014_MISSING_FIELD_OLD_PASS(
    10014,
    "Missing field 'old_pass' or wrong type.",
    "Отсутствует поле 'old_pass' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10015_MISSING_FIELD_NAME(
    10015,
    "Missing field 'name' or wrong type.",
    "Отсутствует поле 'name' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10016_MISSING_FIELD_PASS(
    10016,
    "Missing field 'pass' or wrong type.",
    "Отсутствует поле 'pass' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10017_MISSING_FIELD_ROLE(
    10017,
    "Missing field 'role' or wrong type.",
    "Отсутствует поле 'role' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10018_USER_ALREADY_EXISTS(
    10018,
    "User already exists '$email$' -> '$uuid$'",
    "Пользователь уже существует '$email$' -> '$uuid$'",
    ""
);

static const gtree::ErrorInfo ERR_10019_COULD_NOT_CREATE_USER(
    10019,
    "Could not create user '$email$'",
    "Не удалось создать пользователя '$email$'",
    ""
);

static const gtree::ErrorInfo ERR_10020_MISSING_FIELD_EMAIL(
    10020,
    "Missing field 'email' or wrong type",
    "Отсутствует поле 'email' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10021_COULD_NOT_LOGIN(
    10021,
    "Could not login. Wrong email or password field.",
    "Не удалось войти в систему. Неверный адрес электронной почты или пароль.",
    ""
);

static const gtree::ErrorInfo ERR_10022_MISSING_FIELD_PASS(
    10022,
    "Missing field 'pass' or wrong type.",
    "Отсутствует поле 'pass' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10023_ONLY_ADMIN_CAN_RESET_PASSWORD(
    10023,
    "Only admin can reset password.",
    "Только администратор может сбросить пароль.",
    ""
);

static const gtree::ErrorInfo ERR_10024_MISSING_FIELD_PASS(
    10024,
    "Missing field 'pass' or wrong type",
    "Отсутствует поле 'pass' или имеет неправильный тип.",
    ""
);

static const gtree::ErrorInfo ERR_10025_USER_NOT_FOUND_WITH_EMAIL(
    10025,
    "User not found with email '$email$'.",
    "Пользователь с адресом электронной почты '$email$' не найден.",
    ""
);

static const gtree::ErrorInfo ERR_10026_COULD_NOT_UPDATE_PASSWORD_FOR(
    10026,
    "Could not update password for '$email$', error = '$error$'",
    "Не удалось обновить пароль для '$email$', ошибка = '$error$'",
    ""
);

static const gtree::ErrorInfo ERR_10027_USER_NOT_FOUND_WITH_EMAIL(
    10027,
    "User not found with email '$email$'.",
    "Пользователь с адресом электронной почты '$email$' не найден.",
    ""
);

static const gtree::ErrorInfo ERR_10028_WRONG_PASSWORD(
    10028,
    "Wrong password.",
    "Неверный пароль.",
    ""
);

static const gtree::ErrorInfo ERR_10029_COULD_NOT_UPDATE_PASSWORD_FOR(
    10029,
    "Could not update password for '$email$', error = '$error$'",
    "Не удалось обновить пароль для '$email$', ошибка = '$error$'",
    ""
);