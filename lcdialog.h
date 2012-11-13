#ifndef LCDIALOG_H
#define LCDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QLineEdit>

#include "mainwindow.h"
#include "previewer.h"
#include "recognizer.h"

//-----------------------------------------------------------------

#include <QItemDelegate>
#include <QComboBox>

class ComboBoxDelegate : public QItemDelegate {
public:
    ComboBoxDelegate(const QMap<int, QString> &data, QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QString textForId(int id) const;

private:
    QMap<int, QString> cbData;
};

class DoubleSpinDelegate : public QItemDelegate {
public:
    DoubleSpinDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


//-----------------------------------------------------------------

class ExtTableView : public QTableView {
    Q_OBJECT

public:
    ExtTableView(QWidget *parent = 0);

    void setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate);

protected:
    void keyReleaseEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    bool blockChanges;

private slots:
    void checkForNewRow(QWidget *w);

signals:
    void updateTotal();
};

//-----------------------------------------------------------------

class LCDialog : public QDialog {
    Q_OBJECT

public:
    LCDialog(const QString &chequePath, MainWindow *parent);

    Cheque collectData() const;

private:
    QString cPath;
    ImagePreviewer *ipPreview;
    ExtTableView *tvContent;
    QStandardItemModel *contentModel;
    QPushButton *pbOK, *pbCancel;
    QDateTimeEdit *dteEditor;
    QLineEdit *leSum;

    double getCost(int pos) const;
    void requestRecognition(const QString &filePath);
    void setColor(QModelIndex pos, int val);

private slots:
    void updateTotal();
};

#endif // LCDIALOG_H
