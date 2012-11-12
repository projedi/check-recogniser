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

//-----------------------------------------------------------------

class ExtTableView : public QTableView {
    Q_OBJECT

public:
    ExtTableView(QWidget *parent = 0);

protected:
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    void checkForNewRow();
};

//-----------------------------------------------------------------

class LCDialog : public QDialog {
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

    void requestRecognition(const QString &filePath);
};

#endif // LCDIALOG_H
