/*!
 * \file       chat_lists.cpp
 * \brief      Chat List widget for aiOr app (source file).
 * \details    Contains the Chat List widget class implementation.
 * \author     Arthur Markaryan
 * \date       09.01.2026
 * \copyright  Arthur Markaryan
 */

#include "chatlist.h"
#include "ui_chatlist.h"

ChatList::ChatList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatList)
{
    ui->setupUi(this);
}

ChatList::~ChatList()
{
    delete ui;
}
