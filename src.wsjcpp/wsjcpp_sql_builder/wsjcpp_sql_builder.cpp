/**********************************************************************************
 * MIT License
 *
 * Copyright (c) 2025-2026 Evgenii Sopov <mrseakg@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Official Source Code: https://github.com/wsjcpp/wsjcpp-sql-builder
 *
 ***********************************************************************************/

#include "wsjcpp_sql_builder.h"
#include <algorithm>


namespace wsjcpp {

// ---------------------------------------------------------------------
// SqlBuilderHelpers

std::string SqlBuilderHelpers::escapingStringValue(const std::string &sValue) {
  // escaping simbols NUL (ASCII 0), \n, \r, \, ', ", и Control-Z.
  std::string sResult;
  sResult.reserve(sValue.size() * 2);
  sResult.push_back('\'');
  for (int i = 0; i < sValue.size(); i++) {
    char c = sValue[i];
    if (c == '\n') {
      sResult.push_back('\\');
      sResult.push_back('n');
    } else if (c == '\r') {
      sResult.push_back('\\');
      sResult.push_back('r');
    } else if (c == '\\' || c == '"') {
      sResult.push_back('\\');
      sResult.push_back(c);
    } else if (c == '\'') {
      sResult.push_back('\'');
      sResult.push_back(c);
    } else if (c == 0) {
      sResult.push_back('\\');
      sResult.push_back('0');
    } else {
      sResult.push_back(c);
    }
  }
  sResult.push_back('\'');
  return sResult;
}


SqlQuery::SqlQuery(SqlQueryType sqlType, SqlBuilder *builder, const std::string &tableName)
  : m_sqlType(sqlType), m_builder(builder), m_tableName(tableName) {

}

SqlQueryType SqlQuery::sqlType() {
  return m_sqlType;
}

SqlBuilder &SqlQuery::builder() {
  return *m_builder;
}

SqlBuilder *SqlQuery::builderRawPtr() {
  return m_builder;
}

const std::string &SqlQuery::tableName() {
  return m_tableName;
}

// ---------------------------------------------------------------------
// SqlWhereBase

SqlWhereBase::SqlWhereBase(SqlWhereType type) : m_type(type) {

};

SqlWhereType SqlWhereBase::type() {
  return m_type;
}

// ---------------------------------------------------------------------
// SqlWhereOr

SqlWhereOr::SqlWhereOr() : SqlWhereBase(SqlWhereType::LOGICAL_OPERATOR) { }
std::string SqlWhereOr::sql() { return " OR "; };

// ---------------------------------------------------------------------
// SqlWhereAnd

SqlWhereAnd::SqlWhereAnd() : SqlWhereBase(SqlWhereType::LOGICAL_OPERATOR) { }
std::string SqlWhereAnd::sql() { return " AND "; };

// ---------------------------------------------------------------------
// SqlWhereCondition

SqlWhereCondition::SqlWhereCondition(
  const std::string &name,
  SqlWhereConditionType comparator,
  const std::string &value
) : SqlWhereBase(SqlWhereType::CONDITION), m_name(name), m_comparator(comparator) {
  // TODO in different databases different quotes, mssql have a column names in double quotes
  m_value = SqlBuilderHelpers::escapingStringValue(value);
}

SqlWhereCondition::SqlWhereCondition(
  const std::string &name,
  SqlWhereConditionType comparator,
  int value
) : SqlWhereBase(SqlWhereType::CONDITION), m_name(name), m_comparator(comparator) {
  m_value = std::to_string(value);
}

SqlWhereCondition::SqlWhereCondition(
  const std::string &name,
  SqlWhereConditionType comparator,
  long value
) : SqlWhereBase(SqlWhereType::CONDITION), m_name(name), m_comparator(comparator) {
  m_value = std::to_string(value);
}

SqlWhereCondition::SqlWhereCondition(
  const std::string &name,
  SqlWhereConditionType comparator,
  double value
) : SqlWhereBase(SqlWhereType::CONDITION), m_name(name), m_comparator(comparator) {
  m_value = std::to_string(value);
}

SqlWhereCondition::SqlWhereCondition(
  const std::string &name,
  SqlWhereConditionType comparator,
  float value
) : SqlWhereBase(SqlWhereType::CONDITION), m_name(name), m_comparator(comparator) {
  m_value = std::to_string(value);
}

const std::string &SqlWhereCondition::name() {
  return m_name;
}

SqlWhereConditionType SqlWhereCondition::comparator() {
  return m_comparator;
}

const std::string &SqlWhereCondition::value() {
  return m_value;
}

std::string SqlWhereCondition::sql() {
  std::string ret;
  ret += m_name; // TODO validate and escaping
  switch (m_comparator) {
    case SqlWhereConditionType::NOT_EQUAL:
      ret += " <> ";
      break;
    case SqlWhereConditionType::EQUAL:
      ret += " = ";
      break;
    case SqlWhereConditionType::MORE_THEN:
      ret += " > ";
      break;
    case SqlWhereConditionType::LESS_THEN:
      ret += " < ";
      break;
    case SqlWhereConditionType::LIKE:
      ret += " LIKE ";
      break;
    default:
      ret += " unknwon_operator ";
      break;
  }
  ret += m_value;
  return ret;
}

// ---------------------------------------------------------------------
// SqlSelect

SqlSelect::SqlSelect(const std::string &tableName, SqlBuilder *builder)
: SqlQuery(SqlQueryType::SELECT, builder, tableName) {
  // TODO multitype table names with AS
}

SqlSelect &SqlSelect::colum(const std::string &col, const std::string &col_as) {
  auto it = std::find(m_columns.begin(), m_columns.end(), col);
  if (it != m_columns.end()) {
    builder().addError("Column '" + col + "' already added to select");
  } else {
    m_columns.push_back(col);
    m_columns_as[col] = col_as;
  }
  return *this;
}

SqlWhere<SqlSelect> &SqlSelect::where() {
  if (!m_where) {
    m_where = std::make_shared<SqlWhere<SqlSelect>>(nullptr, builderRawPtr(), this);
  }
  return *(m_where.get());
}

std::string SqlSelect::sql() {
  std::string ret = "SELECT ";
  // TODO TOP OR LIMIT for different databases

  if (m_columns.size() == 0) {
    ret += "*";
  } else {
    bool first = true;
    for (auto col : m_columns) {
      if (!first) {
        ret += ", ";
      }
      ret += col;
      if (m_columns_as[col] != "") {
        ret += " AS " + m_columns_as[col];
      }
      first = false;
    }
    ret += " FROM ";
    ret += tableName();
  }

  if (m_where) {
    ret += " WHERE " + m_where->sql();
  }

  // TODO group by
  // TODO order by
  return ret;
}

// ---------------------------------------------------------------------
// SqlInsert

SqlInsert::SqlInsert(const std::string &tableName, SqlBuilder *builder)
: SqlQuery(SqlQueryType::INSERT, builder, tableName) {

}

SqlInsert &SqlInsert::colum(const std::string &col) {
  m_columns.push_back(col);
  return *this;
}

SqlInsert &SqlInsert::addColums(const std::vector<std::string> &cols) {
  for (auto col : cols) {
    m_columns.push_back(col);
  }
  return *this;
}

SqlInsert &SqlInsert::clearValues() {
  m_values.clear();
  return *this;
}

SqlInsert &SqlInsert::val(const std::string &val) {
  m_values.push_back(SqlBuilderHelpers::escapingStringValue(val));
  return *this;
}

SqlInsert &SqlInsert::val(int val) {
  m_values.push_back(std::to_string(val));
  return *this;
}

SqlInsert &SqlInsert::val(long val) {
  m_values.push_back(std::to_string(val));
  return *this;
}

SqlInsert &SqlInsert::val(float val) {
  m_values.push_back(std::to_string(val));
  return *this;
}

SqlInsert &SqlInsert::val(double val) {
  m_values.push_back(std::to_string(val));
  return *this;
}

std::string SqlInsert::sql() {
  std::string ret = "INSERT INTO " + tableName();

  // TODO if columns is empty
  ret += "(";
  bool first = true;
  for (auto col : m_columns) {
    if (!first) {
      ret += ", ";
    }
    ret += col;
    first = false;
  }
  ret += ")";

  ret += " VALUES(";
  first = true;
  for (auto val : m_values) {
    if (!first) {
      ret += ", ";
    }
    ret += val;
    first = false;
  }
  ret += ")";

  return ret;
};

// ---------------------------------------------------------------------
// SqlUpdate

SqlUpdate::SqlUpdate(const std::string &tableName, SqlBuilder *builder)
  : SqlQuery(SqlQueryType::UPDATE, builder, tableName) {

}

SqlUpdate &SqlUpdate::set(const std::string &name, const std::string &val) {
  return setValue(name, SqlBuilderHelpers::escapingStringValue(val));
}

SqlUpdate &SqlUpdate::set(const std::string &name, int val) {
  return setValue(name, std::to_string(val));
}

SqlUpdate &SqlUpdate::set(const std::string &name, long val) {
  return setValue(name, std::to_string(val));
}

SqlUpdate &SqlUpdate::set(const std::string &name, float val) {
  return setValue(name, std::to_string(val));
}

SqlUpdate &SqlUpdate::set(const std::string &name, double val) {
  return setValue(name, std::to_string(val));
}

SqlUpdate &SqlUpdate::setValue(const std::string &name, const std::string &val) {
  auto it = std::find(m_columns.begin(), m_columns.end(), name);
  if (it != m_columns.end()) {
    m_values[name] = val;
    // builder().addError("Column '" + name + "' already added to select");
  } else {
    m_columns.push_back(name);
    m_values[name] = val;
  }
  return *this;
}

SqlWhere<SqlUpdate> &SqlUpdate::where() {
  if (!m_where) {
    m_where = std::make_shared<SqlWhere<SqlUpdate>>(nullptr, builderRawPtr(), this);
  }
  return *(m_where.get());
}

std::string SqlUpdate::sql() {
  std::string ret = "UPDATE " + tableName() + " SET ";

  // TODO if columns is empty
  bool first = true;
  for (auto col : m_columns) {
    if (!first) {
      ret += ", ";
    }
    ret += col + " = " + m_values[col];
    first = false;
  }

  if (m_where) {
    ret += " WHERE " + m_where->sql();
  }

  return ret;
};


// ---------------------------------------------------------------------
// SqlDelete

SqlDelete::SqlDelete(const std::string &tableName, SqlBuilder *builder)
  : SqlQuery(SqlQueryType::DELETE, builder, tableName) {

}

SqlWhere<SqlDelete> &SqlDelete::where() {
  if (!m_where) {
    m_where = std::make_shared<SqlWhere<SqlDelete>>(nullptr, builderRawPtr(), this);
  }
  return *(m_where.get());
}

std::string SqlDelete::sql() {
  std::string ret = "DELETE FROM " + tableName();

  if (m_where) {
    ret += " WHERE " + m_where->sql();
  }

  return ret;
};

// ---------------------------------------------------------------------
// SqlBuilder

SqlBuilder::SqlBuilder(SqlBuilderForDatabase dbType) : m_dbType(dbType) {

}

SqlSelect &SqlBuilder::selectFrom(const std::string &tableName) {
  m_queries.push_back(std::make_shared<SqlSelect>(tableName, this));
  // TODO check must be select last one;
  return *(SqlSelect *)(m_queries[m_queries.size() -1].get());
}

SqlInsert &SqlBuilder::insertInto(const std::string &tableName) {
  m_queries.push_back(std::make_shared<SqlInsert>(tableName, this));
  return *(SqlInsert *)(m_queries[m_queries.size() -1].get());
}

SqlInsert &SqlBuilder::findInsertOrCreate(const std::string &tableName) {
  for (auto query : m_queries) {
    if (query->sqlType() == SqlQueryType::INSERT && query->tableName() == tableName) {
      return *(SqlInsert *)(query.get());
    }
  }
  return insertInto(tableName);
}

SqlUpdate &SqlBuilder::update(const std::string &tableName) {
  m_queries.push_back(std::make_shared<SqlUpdate>(tableName, this));
  return *(SqlUpdate *)(m_queries[m_queries.size() -1].get());
}

SqlUpdate &SqlBuilder::findUpdateOrCreate(const std::string &tableName) {
  for (auto query : m_queries) {
    if (query->sqlType() == SqlQueryType::UPDATE && query->tableName() == tableName) {
      return *(SqlUpdate *)(query.get());
    }
  }
  return update(tableName);
}

SqlDelete &SqlBuilder::deleteFrom(const std::string &tableName) {
  m_queries.push_back(std::make_shared<SqlDelete>(tableName, this));
  return *(SqlDelete *)(m_queries[m_queries.size() -1].get());
}

SqlDelete &SqlBuilder::findDeleteOrCreate(const std::string &tableName) {
  for (auto query : m_queries) {
    if (query->sqlType() == SqlQueryType::DELETE && query->tableName() == tableName) {
      return *(SqlDelete *)(query.get());
    }
  }
  return deleteFrom(tableName);
}

void SqlBuilder::clear() {
  m_queries.clear();
}

bool SqlBuilder::hasErrors() {
  return m_errors.size() > 0;
}

void SqlBuilder::addError(const std::string &err) {
  m_errors.push_back(err);
}

std::string SqlBuilder::sql() {
  std::string ret = "";
  for (auto query : m_queries) {
    if (ret.size() > 0) {
      ret += "\n";
    }
    ret += query->sql();
  }
  return ret;
}


void SqlBuilder::setDatabaseType(SqlBuilderForDatabase dbType) {
  m_dbType = dbType;
}

SqlBuilderForDatabase SqlBuilder::databaseType() {
  return m_dbType;
}

} // namespace wsjcpp
