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

#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace wsjcpp {

enum class SqlQueryType {
  SELECT,
  INSERT,
  UPDATE,
  DELETE,
};

enum class SqlWhereType {
  LOGICAL_OPERATOR,
  CONDITION,
  SUB_CONDITION,
};

enum class SqlWhereConditionType {
  NOT_EQUAL,
  EQUAL,
  MORE_THEN,
  LESS_THEN,
  LIKE,
};

enum class SqlBuilderForDatabase {
  SQLITE3,
};

class SqlBuilderHelpers {
public:
  static std::string escapingStringValue(const std::string &sValue);
};

class SqlBuilder;
class SqlQuery;
class SqlInsert;
class SqlUpdate;
class SqlSelect;
class SqlDelete;
template<class T> class SqlWhere;

class ISqlBuilder {
public:

  virtual bool hasErrors() = 0;
  virtual std::string sql() = 0;
  virtual void setDatabaseType(SqlBuilderForDatabase dbType) = 0;
  virtual SqlBuilderForDatabase databaseType() = 0;

protected:
  friend SqlWhere<SqlInsert>;
  friend SqlWhere<SqlUpdate>;
  friend SqlWhere<SqlDelete>;
  friend SqlWhere<SqlSelect>;
  friend SqlQuery;
  virtual void addError(const std::string &err) = 0;
};



class SqlQuery {
public:
  SqlQuery(SqlQueryType sqlType, SqlBuilder *builder, const std::string &tableName);
  SqlQueryType sqlType();
  SqlBuilder &builder();
  SqlBuilder *builderRawPtr();
  const std::string &tableName();
  virtual std::string sql() = 0;

private:
  SqlQueryType m_sqlType;
  std::string m_tableName;
  SqlBuilder *m_builder;
};

class SqlWhereBase {
public:
  SqlWhereBase(SqlWhereType type);
  SqlWhereType type();
  virtual std::string sql() = 0;

private:
  SqlWhereType m_type;
};

class SqlWhereOr : public SqlWhereBase {
public:
  SqlWhereOr();
  virtual std::string sql() override;
};

class SqlWhereAnd : public SqlWhereBase {
public:
  SqlWhereAnd();
  virtual std::string sql() override;
};

class SqlWhereCondition : public SqlWhereBase {
public:
  SqlWhereCondition(const std::string &name, SqlWhereConditionType comparator, const std::string &value);
  SqlWhereCondition(const std::string &name, SqlWhereConditionType comparator, int value);
  SqlWhereCondition(const std::string &name, SqlWhereConditionType comparator, long value);
  SqlWhereCondition(const std::string &name, SqlWhereConditionType comparator, double value);
  SqlWhereCondition(const std::string &name, SqlWhereConditionType comparator, float value);
  const std::string &name();
  SqlWhereConditionType comparator();
  const std::string &value();
  virtual std::string sql() override;
private:
  std::string m_name;
  std::string m_value;
  SqlWhereConditionType m_comparator;
};

class SqlSelect;

template<class T>
class SqlWhere : public SqlWhereBase {
public:
  SqlWhere(SqlWhere<T> *parent, SqlBuilder *builder, T *query)
    : SqlWhereBase(SqlWhereType::SUB_CONDITION), m_parent(parent), m_builder(builder), m_query(query) { }

  template <typename TVal>
  SqlWhere<T> &notEqual(const std::string &name, TVal value) {
    cond(name, SqlWhereConditionType::NOT_EQUAL, value);
    return *this;
  }

  template <typename TVal>
  SqlWhere<T> &equal(const std::string &name, TVal value) {
    cond(name, SqlWhereConditionType::EQUAL, value);
    return *this;
  }

  template <typename TVal>
  SqlWhere<T> &moreThen(const std::string &name, TVal value) {
    cond(name, SqlWhereConditionType::MORE_THEN, value);
    return *this;
  }

  template <typename TVal>
  SqlWhere<T> &lessThen(const std::string &name, TVal value) {
    cond(name, SqlWhereConditionType::LESS_THEN, value);
    return *this;
  }

  SqlWhere<T> &like(const std::string &name, const std::string &value) {
    cond(name, SqlWhereConditionType::LIKE, value);
    return *this;
  }

  SqlWhere<T> &or_() {
    if (
      m_conditions.size() > 0
      && m_conditions[m_conditions.size()-1]->type() == SqlWhereType::LOGICAL_OPERATOR
    ) {
      addError("[WARNING] SqlWhere. Last item alredy defined as logical_operator. current will be skipped.");
      return *this;
    }

    m_conditions.push_back(std::make_shared<SqlWhereOr>());
    return *this;
  }

  SqlWhere<T> &and_() {
    if (
      m_conditions.size() > 0
      && m_conditions[m_conditions.size()-1]->type() == SqlWhereType::LOGICAL_OPERATOR
    ) {
      addError("[WARNING] SqlWhere. Last item alredy defined as logical_operator. current will be skipped.");
      return *this;
    }
    m_conditions.push_back(std::make_shared<SqlWhereAnd>());
    return *this;
  }

  SqlWhere<T> &subCondition() {
    if (
      m_conditions.size() > 0
      && m_conditions[m_conditions.size()-1]->type() == SqlWhereType::CONDITION
    ) {
        and_(); // default add and_
    }
    auto sub_cond = std::make_shared<SqlWhere<T>>(this, m_builder, m_query);
    m_conditions.push_back(sub_cond);
    return *(sub_cond.get());
  }

  SqlWhere<T> &finishSubCondition() {
    // TODO return parent
    if (m_parent != nullptr) {
      return *m_parent;
    }
    // default return current where
    // TODO warning to builder
    return *this;
  }

  T &endWhere() {
    return *m_query;
  }

  virtual std::string sql() override {
    std::string ret = "";
    for (auto item : m_conditions) {
      if (item->type() == SqlWhereType::SUB_CONDITION) {
        ret += "(" + item->sql() + ")";
      } else {
        ret += item->sql();
      }
    }
    return ret;
  }

private:
  template <typename TVal>
  SqlWhere<T> &cond(const std::string &name, SqlWhereConditionType comparator, TVal value) {
    if (
      m_conditions.size() > 0
      && m_conditions[m_conditions.size()-1]->type() == SqlWhereType::CONDITION
    ) {
        and_(); // default add and_
    }
    m_conditions.push_back(std::make_shared<SqlWhereCondition>(name, comparator, value));
    return *this;
  }

  void addError(const std::string &err) {
    ((ISqlBuilder *)m_builder)->addError(err);
  }

  SqlBuilder *m_builder;
  T *m_query;
  SqlWhere<T> *m_parent;
  std::vector<std::shared_ptr<SqlWhereBase>> m_conditions;
};

class SqlSelect : public SqlQuery {
public:
  SqlSelect(const std::string &tableName, SqlBuilder *builder);
  SqlSelect &colum(const std::string &col, const std::string &col_as = "");

  SqlWhere<SqlSelect> &where();
  // TODO group by
  // TODO order by
  virtual std::string sql() override;

private:
  std::shared_ptr<SqlWhere<SqlSelect>> m_where;
  std::vector<std::string> m_columns;
  std::map<std::string, std::string> m_columns_as;
};


class SqlInsert : public SqlQuery {
public:
  SqlInsert(const std::string &tableName, SqlBuilder *builder);
  SqlInsert &colum(const std::string &col);
  SqlInsert &addColums(const std::vector<std::string> &cols);
  SqlInsert &clearValues();

  SqlInsert &val(const std::string &val);
  SqlInsert &val(int val);
  SqlInsert &val(long val);
  SqlInsert &val(float val);
  SqlInsert &val(double val);

  virtual std::string sql() override;

private:
  std::vector<std::string> m_columns;
  std::vector<std::string> m_values;
};

class SqlUpdate : public SqlQuery {
public:
  SqlUpdate(const std::string &tableName, SqlBuilder *builder);

  SqlUpdate &set(const std::string &name, const std::string &val);
  SqlUpdate &set(const std::string &name, int val);
  SqlUpdate &set(const std::string &name, long val);
  SqlUpdate &set(const std::string &name, float val);
  SqlUpdate &set(const std::string &name, double val);

  SqlWhere<SqlUpdate> &where();

  virtual std::string sql() override;

private:
  SqlUpdate &setValue(const std::string &name, const std::string &val);

  std::shared_ptr<SqlWhere<SqlUpdate>> m_where;
  std::vector<std::string> m_columns;
  std::map<std::string, std::string> m_values;
};

class SqlDelete : public SqlQuery {
public:
  SqlDelete(const std::string &tableName, SqlBuilder *builder);
  SqlWhere<SqlDelete> &where();
  virtual std::string sql() override;
private:
  std::shared_ptr<SqlWhere<SqlDelete>> m_where;
};

class SqlBuilder : public ISqlBuilder {
public:
  SqlBuilder(SqlBuilderForDatabase dbType = SqlBuilderForDatabase::SQLITE3);
  
  // TODO begin / end transaction can be added here

  SqlSelect &selectFrom(const std::string &tableName);
  SqlInsert &insertInto(const std::string &tableName);
  SqlInsert &findInsertOrCreate(const std::string &tableName);
  SqlUpdate &update(const std::string &tableName);
  SqlUpdate &findUpdateOrCreate(const std::string &tableName);
  SqlDelete &deleteFrom(const std::string &sSqlTable);
  SqlDelete &findDeleteOrCreate(const std::string &tableName);

  void clear();

  virtual bool hasErrors() override;
  virtual std::string sql() override;

  virtual void setDatabaseType(SqlBuilderForDatabase dbType) override;
  virtual SqlBuilderForDatabase databaseType() override;

protected:
  friend SqlSelect;
  friend SqlInsert;
  friend SqlUpdate;
  friend SqlWhere<SqlSelect>;
  virtual void addError(const std::string &err) override;

private:
  std::vector<std::string> m_errors;
  std::vector<std::shared_ptr<SqlQuery>> m_queries;
  SqlBuilderForDatabase m_dbType;
};

} // namespace wsjcpp