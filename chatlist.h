/*!
 * \file       chat_lists.h
 * \brief      Chat List widget for aiOr app (header file).
 * \details    Contains the Chat List widget class and their descriptions.
 * \author     Arthur Markaryan
 * \date       09.01.2026
 * \copyright  Arthur Markaryan
 */

#ifndef _CHATLIST_H_
#define _CHATLIST_H_

#include <QWidget>

namespace Ui {
class ChatList;
}

class ChatList : public QWidget
{
    Q_OBJECT

public:
    explicit ChatList(QWidget *parent = nullptr);
    ~ChatList();

private:
    Ui::ChatList *ui;
};

#endif // _CHATLIST_H_
