#ifndef TABLENEWFIELDDLG_H
#define TABLENEWFIELDDLG_H


#include "ui_TableNewFieldDlg.h"

class TableNewFieldDlg: public QDialog, public Ui::TableNewFieldDlg
{
	Q_OBJECT

public:

    enum ValueType
    {
        SHORT_INTEGER,
        INTERGER,
        CHAR,
        VARCHAR,
        FLOAT,
        DOUBLE,
        TEXT,
        DATE
    };

public:

	TableNewFieldDlg(QDialog *parent = NULL);

	QString getName();

	ValueType getType();

	size_t getLength();

};


#endif //TABLENEWFIELDDLG_H