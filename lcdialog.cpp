#include "lcdialog.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QDoubleValidator>

#include <QDebug>

LCDialog::LCDialog(const QString &chequePath, MainWindow *parent) : QDialog(parent), cPath(chequePath)  {
    ipPreview = new ImagePreviewer(this);
    tvContent = new ExtTableView(this);
    dteEditor = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    leSum = new QLineEdit("0.0", this);
    leSum->setValidator(new QDoubleValidator(0, 100000000.0, 2, this));
    pbOK = new QPushButton(tr("OK"), this);
    pbCancel = new QPushButton(tr("Cancel"), this);
    connect(pbOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(pbCancel, SIGNAL(clicked()), this, SLOT(reject()));

    QLabel *lbDate = new QLabel(tr("<b>Buying date:</b>"), this);
    QLabel *lbTotal = new QLabel(tr("<b>| Total cost: </b>"), this);

    ComboBoxDelegate *cbDelegate = new ComboBoxDelegate(parent->getCategoryNames(), this);

    contentModel = new QStandardItemModel(1, 3, this);
    contentModel->setHeaderData(0, Qt::Horizontal, tr("Good name"), Qt::DisplayRole);
    contentModel->setHeaderData(1, Qt::Horizontal, tr("Category"), Qt::DisplayRole);
    contentModel->setHeaderData(2, Qt::Horizontal, tr("Cost"), Qt::DisplayRole);
    //contentModel->setHeaderData(3, Qt::Horizontal, tr("Quantity"), Qt::DisplayRole);
    //contentModel->setHeaderData(4, Qt::Horizontal, tr("Total"), Qt::DisplayRole);

    tvContent->setModel(contentModel);
    tvContent->setItemDelegateForColumn(1, cbDelegate);
    tvContent->verticalHeader()->hide();
    tvContent->horizontalHeader()->setStretchLastSection(true);
    tvContent->setColumnWidth(0, 200);

    QGridLayout *blayout = new QGridLayout();
    blayout->addWidget(lbDate, 0, 0);
    blayout->addWidget(dteEditor, 0, 1);
    blayout->addWidget(lbTotal, 0, 2);
    blayout->addWidget(leSum, 0, 3);
    blayout->addWidget(pbCancel, 0, 5);
    blayout->addWidget(pbOK, 0, 6);
    blayout->setContentsMargins(0, 0, 0, 0);
    blayout->setSpacing(5);
    blayout->setColumnStretch(4, 1);

    QGridLayout *mlayout = new QGridLayout();
    mlayout->addWidget(ipPreview, 0, 1);
    mlayout->addWidget(tvContent, 0, 0);
    mlayout->addLayout(blayout, 1, 0, 1, 2);
    mlayout->setContentsMargins(5, 5, 5, 5);
    mlayout->setSpacing(5);
    mlayout->setColumnStretch(0, 3);
    mlayout->setColumnStretch(1, 1);
    this->setLayout(mlayout);
    this->setWindowTitle(tr("Adding new cheque"));
    this->resize(700, 450);

    ipPreview->showImage(cPath);
    requestRecognition(cPath);
}

void LCDialog::requestRecognition(const QString &filePath) {
    MainWindow *p = static_cast<MainWindow*>(parent());
    Cheque ch = p->getRecognizer()->recognizeFile(filePath);

    dteEditor->setDateTime(ch.date);
    leSum->setText(QString::number(ch.total));

    contentModel->setRowCount(ch.goods.size()+1);
    for(int i = 0; i < ch.goods.size(); i++) {
        const Good &cg = ch.goods.at(i);
        contentModel->setData(contentModel->index(i, 0), cg.name);
        contentModel->setData(contentModel->index(i, 2), cg.cost);
        int cid = p->getCategoryIDForGood(cg.name);
        if(cid != -1) {
            contentModel->setData(contentModel->index(i, 1), cid, Qt::UserRole);
            contentModel->setData(contentModel->index(i, 1), p->getCategoryNames().value(cid), Qt::DisplayRole);
        }
        //contentModel->setData(contentModel->index(i, 3), cg.count);
        //contentModel->setData(contentModel->index(i, 4), cg.count*cg.cost);
    }
}

Cheque LCDialog::collectData() const {
    Cheque ch;
    ch.date = dteEditor->dateTime();
    ch.total = leSum->text().toDouble();
    for(int i=0; i<contentModel->rowCount()-1; i++) {
        Good g;
        g.name = contentModel->data(contentModel->index(i, 0), Qt::DisplayRole).toString();
        g.category = contentModel->data(contentModel->index(i, 1), Qt::DisplayRole).toString();
        g.cost = contentModel->data(contentModel->index(i, 2), Qt::DisplayRole).toDouble();
        //g.count = contentModel->data(contentModel->index(i, 3)).toDouble();
        g.count = 1;
        ch.goods.append(g);
    }
    return ch;
}

/*************************************************************************/

ExtTableView::ExtTableView(QWidget *parent) : QTableView(parent) {
    connect(itemDelegate(), SIGNAL(closeEditor(QWidget*)), this, SLOT(checkForNewRow()));
}

void ExtTableView::keyReleaseEvent(QKeyEvent *event) {
    QStandardItemModel *tableModel = static_cast<QStandardItemModel*>(model());
    if(!tableModel) return;
    if(event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        QModelIndexList inxs = this->selectedIndexes();
        if(!inxs.isEmpty()) {
            QList<int> rlist;
            foreach(QModelIndex i, inxs) {
                if(!rlist.contains(i.row())) rlist.append(i.row());
            }
            qSort(rlist.begin(), rlist.end());
            if(rlist.last() == tableModel->rowCount()-1 && tableModel->rowCount() > 1) rlist.removeLast();
            for(int i=rlist.size()-1; i>=0; i--) tableModel->removeRow(rlist[i]);
            if(tableModel->rowCount() == 0) tableModel->setRowCount(1);
        }
    }
    QTableView::keyReleaseEvent(event);
}

void ExtTableView::checkForNewRow() {
    QStandardItemModel *tableModel = static_cast<QStandardItemModel*>(model());
    if(!tableModel) return;
    int lastRow = tableModel->rowCount()-1;
    if(!tableModel->data(tableModel->index(lastRow, 0)).toString().isEmpty()) tableModel->setRowCount(lastRow+2);
}

/*************************************************************************/

ComboBoxDelegate::ComboBoxDelegate(const QMap<int, QString> &data, QObject *parent) : QItemDelegate(parent) {
    cbData = data;
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QComboBox *cbEditor = new QComboBox(parent);
    cbEditor->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    cbEditor->setEditable(true);
    for(QMap<int, QString>::const_iterator i = cbData.constBegin(); i != cbData.constEnd(); ++i) {
        cbEditor->addItem(i.value(), i.key());
    }
    cbEditor->installEventFilter(const_cast<ComboBoxDelegate*>(this));
    return cbEditor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    int id = index.model()->data(index, Qt::UserRole).toInt();

    QComboBox *cb = static_cast<QComboBox*>(editor);
    for(int i=0; i<cb->count(); i++) {
        if(cb->itemData(i).toInt() == id) {
            cb->setCurrentIndex(i);
            cb->showPopup();
            break;
        }
    }
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QComboBox *cb = static_cast<QComboBox*>(editor);
    QVariant id = cb->itemData(cb->currentIndex());
    QVariant dt = cb->currentText();
    model->setData(index, id, Qt::UserRole);
    model->setData(index, dt, Qt::DisplayRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry(option.rect);
}

QString ComboBoxDelegate::textForId(int id) const {
    return cbData.value(id);
}
