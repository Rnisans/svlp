#ifndef ABSTRACT_FILTER_H
#define ABSTRACT_FILTER_H

#include <stdint.h>

namespace svlp
{

/*
 * Интерфейс применяемый в SVLP_Parser'е.
 *
 */
struct Abstract_filter
{
    /*
     * Функция check(uint8_t infocode, uint8_t msg_code)
     * Проверяет удовлетворяет ли сообщение с infocode и msg_code, переданными в качестве параметров,
     *   условиям прохождениия фильтра.
     * ret:
     * true - если сообщение удовлетворяет условиям.
     * false - не удовлетворяет
     *
     */
    virtual bool check(uint8_t infocode, uint8_t msg_code) const = 0;
    virtual ~Abstract_filter() {}
};
}

#endif // ABSTRACT_FILTER_H
