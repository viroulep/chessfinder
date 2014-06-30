/*Header file for 'Config Class'
Author: adv (Andrew Marchenko), 03.08.2000
<adv@adv.org.ua>
<t9r@mail.ru>  */
/*
 * Matfinder, a program to help chess engines to find mat
 *
 * Copyright© 2000 Andrew Marchenko
 * Copyright© 2013 Philippe Virouleau
 *
 * You can contact me at firstname.lastname@imag.fr
 * (Replace "firstname" and "lastname" with my actual names)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <fstream>
#include <string>


enum ValType {
    TYPE_EMPTY,
    TYPE_VARIABLE,
    TYPE_KEYWORD,
    TYPE_BLOCK
};

class Value {
    std::string value;
    ValType type;
    int block;

    public:
        Value *next;

        Value();

        void set(std::string str, int bl, ValType key = TYPE_VARIABLE);
        std::string &get();
        const char *sget();

        int is_keyword();
        int is_block();
        int get_block_id();
        ValType get_type();
};


class Config
{
    static const char *defaultConfigFile;
    std::ifstream file;
    char delimiter;
    int vcol;

    Value *svalue, *vstart;

    int open_config_file(const char *name);
    int close_config_file();
    int read_config_file();

    void S_trunc(std::string &str);
    int S_count(const std::string &s, char c);

    int search_block(const char *block);
    const char *search_value(int col);

    public:
        Config();
        Config(const char *name, int col = 1);
        Config(char delim, int col = 1);
        Config(const char *name, char delim, int col = 1);
        int init(const char *name, char delim, int col);

        ~Config();

        const char *value(const char *name, int col = 1);
        const char *value(const char *block, const char *name, int col = 1);
        const char *operator()(const char *name, int col = 1);
        const char *operator()(const char *block, const char *name, int col = 1);

        Value *ikey(const char* block, int index);
        const char *_ivalue(const char *block, const char *name, int index, int col);
        const char *ivalue(int index, int col = 1);
        const char *ivalue(const char *block, int index, int col = 1);
        const char *ivalue(const char *block, const char *name, int index, int col = 1);

        const char *keyword(int index);
};
