/* 
 * This file is part of the ZeusiRCd distribution (https://github.com/Pryancito/zeusircd).
 * Copyright (c) 2019 Rodrigo Santidrian AKA Pryan.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * This file is based on some library from github it's already deleted :)
*/

#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <exception>
#include <memory>
#include <iostream>

namespace sqlite
{
    class sqlite;
    class statement;
    typedef std::shared_ptr<statement> statement_ptr;
    typedef std::shared_ptr<sqlite> sqlite_ptr;
    
    class exception : public std::exception
    {
        friend statement;
        friend sqlite;
    public:
        exception(std::string msg)
        {
            this->_msg = msg;
			std::cout << "Error en SQLite: " << msg << std::endl;
        }
    private:
        void _set_errorno(int err)
        {
        }
        std::string _msg;
    };

    class statement
    {
        friend sqlite;
        friend exception;
    public:
        void set_sql(std::string sql)
        {
            if(this->_prepared)
            {
                exception e("Can not set sql on prepared query.");
                throw e;
            }
            else
            {
                this->_sql = sql;
            }
        }
        void prepare()
        {
            this->_prepared = true;
            const char* tail;
            int rc = sqlite3_prepare_v2(this->_db, 
                    this->_sql.c_str(), 
                    this->_sql.length(), 
                    &this->_s, 
                    &tail);
            if(rc != SQLITE_OK)
            {
                exception e("Could not prepare sql.");
                e._set_errorno(rc);
                throw e;
            }
            this->_tail = std::string(tail);
        }

        bool step()
        {
            if(!this->_valid)
            {
                exception e("Trying to step an invalid statement.");
            }

            int rc = sqlite3_step(this->_s);
            if(rc == SQLITE_DONE)
            {
                this->_valid = false;
                return false;
            }
            if(rc == SQLITE_ROW)
            {
                this->_has_row = true;
                return true;
            }
            // Ok, this means error if we get here
            exception e("Sqlite had an error: " + std::string(sqlite3_errmsg(this->_db)));
            return false;
        }
        void reset()
        {
            int rc = sqlite3_reset(this->_s);
            if(rc != SQLITE_OK)
            {
                exception e("Could not reset the virtual machine.");
                throw e;
            }
            this->_valid = true;
            this->_has_row = false;
            this->_prepared = false;
        }

        void exec()
        {
            this->prepare();
            this->step();
        }

        double get_double(int fieldnumber)
        {
            return sqlite3_column_double(this->_s, fieldnumber);
        }

		int get_columns()
		{
			return sqlite3_column_count(this->_s);
		}

        int get_int(int fieldnumber)
        {
            return sqlite3_column_int(this->_s, fieldnumber);
        }

        std::string get_text(int fieldnumber)
        {
			const char* result = reinterpret_cast<const char*>(sqlite3_column_text(this->_s, fieldnumber));
			return std::string(result?result:"");
        }
        std::string get_blob(int fieldnumber)
        {
            return std::string((const char*)sqlite3_column_blob(this->_s, fieldnumber), 
                    sqlite3_column_bytes(this->_s, fieldnumber));
        }

        void bind(int where, const std::string& text)
        {
            int rc = sqlite3_bind_text(this->_s, where, text.c_str(), text.length(), SQLITE_STATIC);
            if(rc != SQLITE_OK)
            {
                exception e("Could not bind text.");
                throw e;
            }
        }

        void bind(int where, const std::string&& text)
        {
            int rc = sqlite3_bind_text(this->_s, where, text.c_str(), text.length(), SQLITE_TRANSIENT);
            if(rc != SQLITE_OK)
            {
                exception e("Could not bind text.");
                throw e;
            }
        }

        void bind(int where, double d)
        {
            int rc = sqlite3_bind_double(this->_s, where, d);
            if(rc != SQLITE_OK)
            {
                exception e("Could not bind double.");
                throw e;
            }
        }
        void bind(int where, int i)
        {
            int rc = sqlite3_bind_int(this->_s, where, i);
            if(rc != SQLITE_OK)
            {
                exception e("Could not bind int.");
                throw e;
            }
        }
        void bind_null(int where)
        {
            int rc = sqlite3_bind_null(this->_s, where);
            if(rc != SQLITE_OK)
            {
                exception e("Could not bind to NULL.");
                throw e;
            }
        }

        virtual ~statement()
        {
            sqlite3_finalize(this->_s);
        }
    private:
        statement(sqlite3* db)
        {
            this->_db = db;
            this->_prepared = false;
            this->_valid = true;
            this->_has_row = false;
        }
        statement(sqlite3* db, std::string sql)
        {
            this->_db = db;
            this->_prepared = false;
            this->_valid = true;
            this->_has_row = false;
            this->set_sql(sql);
        }
        sqlite3* _db;
        bool _prepared, _valid, _has_row;
        std::string _sql;
        std::string _tail;
        sqlite3_stmt* _s;
    };

    class sqlite
    {
        friend statement;
    public:
        sqlite(std::string filename, bool readonly)
        {
            this->_filename = filename;
            int rc;
            if (readonly == true)
				rc = sqlite3_open_v2(filename.c_str(), &this->_db, SQLITE_OPEN_READONLY, NULL);
			else
				rc = sqlite3_open_v2(filename.c_str(), &this->_db, SQLITE_OPEN_READWRITE, NULL);
            if(rc != SQLITE_OK)
            {
                exception e("Could not open '" + filename + "'");
                throw e;
            }
        }
        std::shared_ptr<statement> get_statement()
        {
            statement_ptr st(new statement(this->_db));
            return st;
        }
        statement_ptr get_statement(std::string sql)
        {
            statement_ptr st(new statement(this->_db, sql));
            return st;
        }
        std::string get_error()
        {
			return sqlite3_errmsg(this->_db);
		}
        int64_t last_insert_id()
        {
            return sqlite3_last_insert_rowid(this->_db);
        }
        virtual ~sqlite()
        {
            sqlite3_close(this->_db);
        }
    private:
        std::string _filename;

        sqlite3* _db;
    };
}
