/**
 * @file        chatlist.cpp
 * @brief       Chat List widget for aiOr application (source file).
 * @details     Contains the Chat List widget class implementation.
 *              Provides a list view for displaying and managing chat sessions.
 *
 * @author      Arthur Markaryan
 * @date        08.05.2026
 * @version     1.0.1
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - Qt5/6 Core (QWidget)
 * - Ui::ChatList (generated from .ui file)
 *
 * @par ChangeLog:
 * 08.05.2026   v1.0.1  Arthur Markaryan - Modify header of the file, add comment
 * 09.01.2026   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         ChatList::ChatList()
 * @see         ChatList::~ChatList()
 * @see         QWidget
 */

#include "chatlist.h"
#include "ui_chatlist.h"

/**
 * @brief       Constructor for ChatList widget.
 * @param       parent  Parent widget (default is nullptr)
 * @details     Initializes the user interface and sets up the chat list widget.
 */
ChatList::ChatList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatList)
{
    ui->setupUi(this);
}

/**
 * @brief       Destructor for ChatList widget.
 * @details     Cleans up allocated UI resources.
 */
ChatList::~ChatList()
{
    delete ui;
}
