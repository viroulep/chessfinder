/*
 * Matfinder, a program to help chess engines to find mat
 *
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

#ifndef __CONCURRENTMAP_H__
#define __CONCURRENTMAP_H__

#include <map>
#include <mutex>

/* Note : This naive lock implementation may be to restrictive and non scalable
 * Rewriting it may be on the todo list some day
 * Note on iterator : they are not threadsafe, despite the name of the class...
 **/

template <typename K, typename V>
class ConcurrentMap : protected std::map<K, V> {
    /*'Protected' because I want the derived class to be able to bypass locks,
     * but I don't want any other class to do that.
     **/
    public:
        const V &findVal(const K &key, const V &defVal = V(), bool insertDef = false);
        const V &findOrInsert(const K &key, const V &value);
        size_t remove(const K &key);
        bool modified();
        size_t size();
        size_t count(const K &key);
        V &operator[](const K &key);
        V &operator[](K &&key);
        typename std::map<K, V>::iterator begin();
        typename std::map<K, V>::iterator end();
    protected:
        std::mutex lock_;
        bool modified_ = false;
};

template <typename K, typename V>
const V &ConcurrentMap<K, V>::findVal(const K &key, const V &defVal, bool insertDef)
{
    std::unique_lock<std::mutex> lock(lock_);
    auto found = this->find(key);
    if (found != this->end()) {
        return found->second;
    }
    if (insertDef) {
        this->insert(std::make_pair(key, defVal));
        modified_ = true;
    }
    return defVal;
}

template <typename K, typename V>
const V &ConcurrentMap<K, V>::findOrInsert(const K &key, const V &value)
{
    return findVal(key, value, true);
}

template <typename K, typename V>
size_t ConcurrentMap<K, V>::remove(const K &key)
{
    std::unique_lock<std::mutex> lock(lock_);
    return this->erase(key);
}

template <typename K, typename V>
size_t ConcurrentMap<K, V>::size()
{
    return std::map<K, V>::size();
}

template <typename K, typename V>
size_t ConcurrentMap<K, V>::count(const K &key)
{
    return std::map<K, V>::count(key);
}

template <typename K, typename V>
V &ConcurrentMap<K, V>::operator[](const K &key)
{
    return std::map<K, V>::operator[](key);
}

template <typename K, typename V>
V &ConcurrentMap<K, V>::operator[](K &&key)
{
    return std::map<K, V>::operator[](key);
}

template <typename K, typename V>
typename std::map<K, V>::iterator ConcurrentMap<K, V>::begin()
{
    return std::map<K, V>::begin();
}

template <typename K, typename V>
typename std::map<K, V>::iterator ConcurrentMap<K, V>::end()
{
    return std::map<K, V>::end();
}
#endif
