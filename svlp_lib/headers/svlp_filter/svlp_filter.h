#ifndef SVLP_FILTER_H
#define SVLP_FILTER_H
#include "abstract_filter.h"
#include "stdint.h"
#include <stddef.h>

namespace svlp
{

/*
 * Интерфейс представляющий собой фильтр, с возможностью добавления и удаления правил фильтрации.
 *     Каждый фильтр состоит из набара элементов, и правил, применяемых к этим элементам.
 *     См. Тип фильтра.
 *
 *
 */
struct Filter_model : public Abstract_filter
{

    /*
     * При добавлении элемента фильтра, ему присваивается идентификатор. Тип идентификатора - uid_t.
     *
     */
    typedef uint8_t uid_t;

    /*
     * Тип фильтра.
     *  blacklist - непропускать указанные
     *  whitelist - пропускать ТОЛЬКО указанные
     */
    enum Type
    {
        blacklist,
        whitelist,
        invalid
    };

    /*
     * Тип элемента фильтра.
     *
     */
    struct Item
    {
        uint8_t info_code;
        uint8_t msg_code;
    };

    /*
     * Функция добавляет элемент к фильтру.
     * ret:
     * Идентификатор добавленного элемента. Данный идентификатор в последствии можно использовать для удаления элемента.
     *
     */
    virtual uid_t insert(Item& item) = 0;

    /*
     * Функция удаления элемента из списка.
     * ret:
     * Результат удаления
     *  true - элемент удалён
     *  false - элемент не был удалён.
     *
     */
    virtual bool remove(const uid_t uid) = 0;

    /*
     * Получить колличество элементов в фильтре.
     *
     */
    virtual size_t length() const = 0;

    /*
     * Очистить фильтр от элементов.
     * Тип фильтра при этом не меняется.
     *
     */
    virtual void clear() = 0;

    /*
     * Получить элемент под указаным индексом
     *
     */
    virtual Item at(size_t index) const = 0;

    virtual Type getType() const = 0;
    virtual void setType(const Type& value) = 0;

    virtual ~Filter_model();
};

class Simple_filter : public Filter_model
{
public:
    Simple_filter(Type type);
    Simple_filter();
    uid_t insert(Item& item);

    /*
     * Функция удаления элемента из списка.
     * ret:
     * Результат удаления
     *  true - элемент удалён
     *  false - элемент не был удалён.
     *
     * Notice: удалять элемент с определённым uid можно только один раз. Повторный вызов функции remove, с одним и тем же uid приводит к неопределённому поведению.
     *
     */

    bool remove(const uid_t uid);

    /*
     * Проверить удовлетворяет ли сообщение с указанными infocode и msg_code условиям прохождения фильтра.
     *    учитывается тип фильтра и все элементы.
     * ret:
     * Результат проверки
     *  true - если сообщение удовлетворяет условиям.
     *  false - не удовлетворяет
     */
    bool check(uint8_t infocode, uint8_t msg_code) const;

    /*
     * Получить колличество элементов в списке
     * ret:
     *   Колличество эллементов в списке.
     */
    size_t length() const;

    /*
     * Очистить фильтр от элементов.
     * Тип фильтра при этом не меняется.
     *
     */
    void clear();

    /*
     * Получить элемент под указаным индексом
     *
     */
    Item at(size_t index) const;

    Type getType() const;
    void setType(const Type& value);

private:
    /*
     * Проверка наличия записи с указанными infocode и msg_code  в списке элементов
     * Результат проверки зависит от наличия записи в списке и типа фильтра (whitelist, blacklist)
     */
    bool is_list_contain(uint8_t infocode, uint8_t msgcode) const;

    /*
     * Тип списка:
     *    whitelist - пропускать только сообщения, которые имеются в списке
     *    blacklist - пропускать все сообщения, кроме тех, которые указаны в списке.
     */
    Type type;

    /*
     * Список элементов  на основе массива.
     */
    Item list[16];

    /*
     * Указывает на первый свободный индекс.
     */
    size_t list_length;
};
}
#endif // SVLP_FILTER_H
