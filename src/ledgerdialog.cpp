/***************************************************************************
 *   Copyright (C) 2006-2008, 2014, 2016 by Hanna Knutsson                 *
 *   hanna_k@fmgirl.com                                                    *
 *                                                                         *
 *   This file is part of Eqonomize!.                                      *
 *                                                                         *
 *   Eqonomize! is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Eqonomize! is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with Eqonomize!. If not, see <http://www.gnu.org/licenses/>.    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ledgerdialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QMenu>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollBar>
#include <QStringList>
#include <QTextStream>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QAction>
#include <QUrl>
#include <QFileDialog>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QMimeDatabase>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QSettings>
#include <QHeaderView>

#include "budget.h"
#include "eqonomize.h"
#include "transactioneditwidget.h"

extern QString htmlize_string(QString str);
extern QColor createExpenseColor(QTreeWidgetItem *i, int = 0);
extern QColor createIncomeColor(QTreeWidgetItem *i, int = 0);
extern QColor createTransferColor(QTreeWidgetItem *i, int = 0);
extern void setColumnTextWidth(QTreeWidget *w, int i, QString str);
extern void setColumnDateWidth(QTreeWidget *w, int i);
extern void setColumnMoneyWidth(QTreeWidget *w, int i, double v = 9999999.99);
extern void setColumnValueWidth(QTreeWidget *w, int i, double v, int d = -1);
extern void setColumnStrlenWidth(QTreeWidget *w, int i, int l);


QColor incomeColor, expenseColor;

class LedgerListViewItem : public QTreeWidgetItem {
	protected:
		Transaction *o_trans;
		SplitTransaction *o_split;
	public:
		LedgerListViewItem(Transaction *trans, SplitTransaction *split, QTreeWidget *parent, QString, QString=QString::null, QString=QString::null, QString=QString::null, QString=QString::null, QString=QString::null, QString=QString::null, QString=QString::null);
		void setColors();
		Transaction *transaction() const;		
		SplitTransaction *splitTransaction() const;
};

LedgerListViewItem::LedgerListViewItem(Transaction *trans, SplitTransaction *split, QTreeWidget *parent, QString s1, QString s2, QString s3, QString s4, QString s5, QString s6, QString s7, QString s8) : QTreeWidgetItem(parent), o_trans(trans), o_split(split) {
	setText(0, s1);
	setText(1, s2);
	setText(2, s3);
	setText(3, s4);
	setText(5, s5);
	setText(6, s6);
	setText(7, s7);
	setText(4, s8);
	setTextAlignment(4, Qt::AlignRight | Qt::AlignVCenter);
	setTextAlignment(5, Qt::AlignRight | Qt::AlignVCenter);
	setTextAlignment(6, Qt::AlignRight | Qt::AlignVCenter);
	setTextAlignment(7, Qt::AlignRight | Qt::AlignVCenter);
	if(parent) {
		if(!expenseColor.isValid()) expenseColor = createExpenseColor(this, 4);
		setForeground(4, expenseColor);
		if(!incomeColor.isValid()) incomeColor = createIncomeColor(this, 5);
		setForeground(5, incomeColor);
		setForeground(6, expenseColor);
	}
}
void LedgerListViewItem::setColors() { 
	if(!expenseColor.isValid()) expenseColor = createExpenseColor(this, 4);
	setForeground(4, expenseColor);
	if(!incomeColor.isValid()) incomeColor = createIncomeColor(this, 5);
	setForeground(5, incomeColor);
	setForeground(6, expenseColor);
}
Transaction *LedgerListViewItem::transaction() const {
	return o_trans;
}
SplitTransaction *LedgerListViewItem::splitTransaction() const {
	return o_split;
}

LedgerDialog::LedgerDialog(AssetsAccount *acc, Eqonomize *parent, QString title, bool extra_parameters) : QDialog(NULL, 0), account(acc), mainWin(parent), b_extra(extra_parameters) {

	setWindowTitle(title);
	setModal(true);

	setAttribute(Qt::WA_DeleteOnClose, true);

	budget = account->budget();

	QVBoxLayout *box1 = new QVBoxLayout(this);

	QHBoxLayout *topbox = new QHBoxLayout();
	box1->addLayout(topbox);
	topbox->addWidget(new QLabel(tr("Account:"), this));
	accountCombo = new QComboBox(this);
	accountCombo->setEditable(false);
	int i = 0;
	AssetsAccount *aaccount = budget->assetsAccounts.first();
	while(aaccount) {
		if(aaccount != budget->balancingAccount && aaccount->accountType() != ASSETS_TYPE_SECURITIES) {
			accountCombo->addItem(aaccount->name());
			if(aaccount == account) accountCombo->setCurrentIndex(i);
			i++;
		}
		aaccount = budget->assetsAccounts.next();
	}
	topbox->addWidget(accountCombo);
	QDialogButtonBox *topbuttons = new QDialogButtonBox(this);
	exportButton = topbuttons->addButton(tr("Export…"), QDialogButtonBox::ActionRole);
	printButton = topbuttons->addButton(tr("Print…"), QDialogButtonBox::ActionRole);
	topbox->addWidget(topbuttons);
	topbox->addStretch(1);

	QHBoxLayout *box2 = new QHBoxLayout();
	box1->addLayout(box2);
	QVBoxLayout *box3 = new QVBoxLayout();
	box2->addLayout(box3);
	transactionsView = new EqonomizeTreeWidget(this);
	transactionsView->setSortingEnabled(false);
	transactionsView->setAllColumnsShowFocus(true);
	transactionsView->setColumnCount(8);
	QStringList headers;
	headers << tr("Date");
	headers << tr("Type");
	headers << tr("Description", "Transaction description property (transaction title/generic article name)");
	headers << tr("Account/Category");
	headers << tr("Expense");
	headers << tr("Deposit");
	headers << tr("Withdrawal");
	headers << tr("Balance", "Noun. Balance of an account");
	transactionsView->setHeaderLabels(headers);
	transactionsView->setRootIsDecorated(false);
	setColumnDateWidth(transactionsView, 0);
	setColumnStrlenWidth(transactionsView, 1, 15);
	setColumnStrlenWidth(transactionsView, 2, 25);
	setColumnStrlenWidth(transactionsView, 3, 20);
	setColumnMoneyWidth(transactionsView, 4);
	setColumnMoneyWidth(transactionsView, 5);
	setColumnMoneyWidth(transactionsView, 6);
	setColumnMoneyWidth(transactionsView, 7, 999999999999.99);
	transactionsView->setSelectionMode(QTreeWidget::ExtendedSelection);
	QSizePolicy sp = transactionsView->sizePolicy();
	sp.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
	transactionsView->setSizePolicy(sp);
	box3->addWidget(transactionsView);
	QDialogButtonBox *buttons = new QDialogButtonBox(Qt::Vertical, this);
	QPushButton *newButton = buttons->addButton(tr("New"), QDialogButtonBox::ActionRole);
	QMenu *newMenu = new QMenu(this);
	newButton->setMenu(newMenu);
	ActionNewDebtPayment = newMenu->addAction(mainWin->ActionNewDebtPayment->icon(), mainWin->ActionNewDebtPayment->text());
	connect(ActionNewDebtPayment, SIGNAL(triggered()), this, SLOT(newDebtPayment()));
	ActionNewDebtInterest = newMenu->addAction(mainWin->ActionNewDebtInterest->icon(), mainWin->ActionNewDebtInterest->text());
	connect(ActionNewDebtInterest, SIGNAL(triggered()), this, SLOT(newDebtInterest()));
	connect(newMenu->addAction(mainWin->ActionNewExpense->icon(), mainWin->ActionNewExpense->text()), SIGNAL(triggered()), this, SLOT(newExpense()));
	connect(newMenu->addAction(mainWin->ActionNewIncome->icon(), mainWin->ActionNewIncome->text()), SIGNAL(triggered()), this, SLOT(newIncome()));
	connect(newMenu->addAction(mainWin->ActionNewTransfer->icon(), mainWin->ActionNewTransfer->text()), SIGNAL(triggered()), this, SLOT(newTransfer()));
	connect(newMenu->addAction(mainWin->ActionNewMultiItemTransaction->icon(), mainWin->ActionNewMultiItemTransaction->text()), SIGNAL(triggered()), this, SLOT(newSplit()));
	editButton = buttons->addButton(tr("Edit…"), QDialogButtonBox::ActionRole);
	editButton->setEnabled(false);
	removeButton = buttons->addButton(tr("Delete"), QDialogButtonBox::ActionRole);
	removeButton->setEnabled(false);
	joinButton = buttons->addButton(tr("Join…"), QDialogButtonBox::ActionRole);
	joinButton->setEnabled(false);
	splitUpButton = buttons->addButton(tr("Split Up"), QDialogButtonBox::ActionRole);
	splitUpButton->setEnabled(false);
	box2->addWidget(buttons);
	
	statLabel = new QLabel("", this);
	box3->addWidget(statLabel);
	
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
	buttonBox->button(QDialogButtonBox::Close)->setShortcut(Qt::CTRL | Qt::Key_Return);
	buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
	connect(buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(reject()));
	box1->addWidget(buttonBox);

	connect(transactionsView, SIGNAL(itemSelectionChanged()), this, SLOT(transactionSelectionChanged()));
	connect(transactionsView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(edit(QTreeWidgetItem*)));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
	connect(editButton, SIGNAL(clicked()), this, SLOT(edit()));
	connect(joinButton, SIGNAL(clicked()), this, SLOT(joinTransactions()));
	connect(splitUpButton, SIGNAL(clicked()), this, SLOT(splitUpTransaction()));
	connect(exportButton, SIGNAL(clicked()), this, SLOT(saveView()));
	connect(printButton, SIGNAL(clicked()), this, SLOT(printView()));
	connect(accountCombo, SIGNAL(activated(int)), this, SLOT(accountActivated(int)));
	connect(mainWin, SIGNAL(transactionsModified()), this, SLOT(updateTransactions()));
	connect(mainWin, SIGNAL(accountsModified()), this, SLOT(updateAccounts()));

	accountChanged();
	
	//if(!transactionsView->topLevelItem(0)) 
	transactionsView->setMinimumWidth(transactionsView->columnWidth(0) + transactionsView->columnWidth(1) + transactionsView->columnWidth(2) +  transactionsView->columnWidth(3) +  transactionsView->columnWidth(4) +  transactionsView->columnWidth(5) +  transactionsView->columnWidth(6) + 10);
	//else transactionsView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
	
	QSettings settings;
	QSize dialog_size = settings.value("Ledger/size", QSize()).toSize();
	if(dialog_size.isValid()) {
		resize(dialog_size);
	}

}
LedgerDialog::~LedgerDialog() {}

void LedgerDialog::saveConfig() {
	QSettings settings;
	settings.setValue("Ledger/size", size());
}

void LedgerDialog::accountChanged() {
	if(!account) return;
	bool b_loan = (account->accountType() == ASSETS_TYPE_LIABILITIES || account->accountType() == ASSETS_TYPE_CREDIT_CARD);
	transactionsView->setColumnHidden(4, !b_loan);
	ActionNewDebtInterest->setVisible(b_loan);
	ActionNewDebtPayment->setVisible(b_loan);
	updateTransactions();
}
void LedgerDialog::accountActivated(int index) {
	int i = 0;
	AssetsAccount *aaccount = budget->assetsAccounts.first();
	while(aaccount) {
		if(aaccount != budget->balancingAccount && aaccount->accountType() != ASSETS_TYPE_SECURITIES) {
			if(i == index) {
				account = aaccount;
				break;
			}
			i++;
		}
		aaccount = budget->assetsAccounts.next();
	}
	accountChanged();
}
void LedgerDialog::updateAccounts() {
	accountCombo->blockSignals(true);
	int i = 0;
	bool account_found = false;
	AssetsAccount *aaccount = budget->assetsAccounts.first();
	while(aaccount) {
		if(aaccount != budget->balancingAccount && aaccount->accountType() != ASSETS_TYPE_SECURITIES) {
			accountCombo->addItem(aaccount->name());
			if(aaccount == account) {
				accountCombo->setCurrentIndex(i);
				account_found = true;
			}
			i++;
		}
		aaccount = budget->assetsAccounts.next();
	}
	if(!account_found) {
		if(accountCombo->count() == 0) {
			close();
			return;
		}
		accountActivated(0);
	} else {
		updateTransactions();
	}
	accountCombo->blockSignals(false);
}
void LedgerDialog::saveView() {
	if(transactionsView->topLevelItemCount() == 0) {
		QMessageBox::critical(this, tr("Error"), tr("Empty transaction list."));
		return;
	}
	char filetype = 'h';
	QMimeDatabase db;
	QString html_filter = db.mimeTypeForName("text/html").filterString();
	QString csv_filter = db.mimeTypeForName("text/csv").filterString();
	QString filter = html_filter;
	QUrl url = QFileDialog::getSaveFileUrl(this, QString(), QUrl(), html_filter + ";;" + csv_filter, &filter);
	if(filter == csv_filter) filetype = 'c';
	if(url.isEmpty() || !url.isValid()) return;
	QSaveFile ofile(url.toLocalFile());
	ofile.open(QIODevice::WriteOnly);
	ofile.setPermissions((QFile::Permissions) 0x0660);
	if(!ofile.isOpen()) {
		ofile.cancelWriting();
		QMessageBox::critical(this, tr("Error"), tr("Couldn't open file for writing."));
		return;
	}
	QTextStream stream(&ofile);
	exportList(stream, filetype);
	if(!ofile.commit()) {
		QMessageBox::critical(this, tr("Error"), tr("Error while writing file; file was not saved."));
		return;
	}

}
bool LedgerDialog::exportList(QTextStream &outf, int fileformat) {

	switch(fileformat) {
		case 'h': {
			outf.setCodec("UTF-8");
			outf << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">" << '\n';
			outf << "<html>" << '\n';
			outf << "\t<head>" << '\n';
			outf << "\t\t<title>"; outf << htmlize_string(tr("Ledger")); outf << "</title>" << '\n';
			outf << "\t\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">" << '\n';
			outf << "\t\t<meta name=\"GENERATOR\" content=\"" << qApp->applicationDisplayName() << "\">" << '\n';
			outf << "\t</head>" << '\n';
			outf << "\t<body>" << '\n';
			outf << "\t\t<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\">" << '\n';
			outf << "\t\t\t<caption>"; outf << htmlize_string(tr("Transactions for %1").arg(account->name())); outf << "</caption>" << '\n';
			outf << "\t\t\t<thead>" << '\n';
			outf << "\t\t\t\t<tr>" << '\n';
			QTreeWidgetItem *header = transactionsView->headerItem();
			outf << "\t\t\t\t\t<th>" << htmlize_string(header->text(0)) << "</th>";
			outf << "<th>" << htmlize_string(header->text(1)) << "</th>";
			outf << "<th>" << htmlize_string(header->text(2)) << "</th>";
			outf << "<th>" << htmlize_string(header->text(3)) << "</th>";
			outf << "<th>" << htmlize_string(header->text(4)) << "</th>";
			outf << "<th>" << htmlize_string(header->text(5)) << "</th>";
			outf << "<th>" << htmlize_string(header->text(6)) << "</th>" << "\n";
			outf << "\t\t\t\t</tr>" << '\n';
			outf << "\t\t\t</thead>" << '\n';
			outf << "\t\t\t<tbody>" << '\n';
			QTreeWidgetItemIterator it(transactionsView);
			LedgerListViewItem *i = (LedgerListViewItem*) *it;
			while(i) {
				outf << "\t\t\t\t<tr>" << '\n';
				outf << "\t\t\t\t\t<td nowrap align=\"right\">" << htmlize_string(i->text(0)) << "</td>";
				outf << "<td>" << htmlize_string(i->text(1)) << "</td>";
				outf << "<td>" << htmlize_string(i->text(2)) << "</td>";
				outf << "<td>" << htmlize_string(i->text(3)) << "</td>";
				outf << "<td nowrap align=\"right\">" << htmlize_string(i->text(4)) << "</td>";
				outf << "<td nowrap align=\"right\">" << htmlize_string(i->text(5)) << "</td>";
				outf << "<td nowrap align=\"right\">" << htmlize_string(i->text(6)) << "</td>" << "\n";
				outf << "\t\t\t\t</tr>" << '\n';
				++it;
				i = (LedgerListViewItem*) *it;
			}
			outf << "\t\t\t</tbody>" << '\n';
			outf << "\t\t</table>" << '\n';
			outf << "\t</body>" << '\n';
			outf << "</html>" << '\n';
			break;
		}
		case 'c': {
			//outf.setEncoding(Q3TextStream::Locale);
			QTreeWidgetItem *header = transactionsView->headerItem();
			outf << "\"" << header->text(0) << "\",\"" << header->text(1) << "\",\"" << header->text(2) << "\",\"" << header->text(3) << "\",\"" << header->text(4) << "\",\""<< header->text(5) << "\",\"" << header->text(6) << "\"\n";
			QTreeWidgetItemIterator it(transactionsView);
			LedgerListViewItem *i = (LedgerListViewItem*) *it;
			while(i) {
				outf << "\"" << i->text(0) << "\",\"" << i->text(1) << "\",\"" << i->text(2) << "\",\"" << i->text(3) << "\",\"" << i->text(4) << "\",\"" << i->text(5) << "\",\"" << i->text(6) << "\"\n";
				++it;
				i = (LedgerListViewItem*) *it;
			}
			break;
		}
	}

	return true;

}
void LedgerDialog::printView() {
	if(transactionsView->topLevelItemCount() == 0) {
		QMessageBox::critical(this, tr("Error"), tr("Empty transaction list."));
		return;
	}
	QPrinter printer;
	QPrintDialog print_dialog(&printer, this);
	if(print_dialog.exec() == QDialog::Accepted) {
		QString str;
		QTextStream stream(&str, QIODevice::WriteOnly);
		exportList(stream, 'h');
		QTextDocument htmldoc;
		htmldoc.setHtml(str);
		htmldoc.print(&printer);
	}
}
void LedgerDialog::joinTransactions() {
	QList<QTreeWidgetItem*> selection = transactionsView->selectedItems();
	MultiItemTransaction *split = NULL;
	QString payee;
	for(int index = 0; index < selection.size(); index++) {
		LedgerListViewItem *i = (LedgerListViewItem*) selection[index];
		if(!i->splitTransaction()) {
			Transaction *trans = i->transaction();
			if(trans && !trans->parentSplit()) {
				if(!split) {
					split = new MultiItemTransaction(budget, i->transaction()->date(), account);
				}
				if(payee.isEmpty()) {
					if(trans->type() == TRANSACTION_TYPE_EXPENSE) payee = ((Expense*) trans)->payee();
					else if(trans->type() == TRANSACTION_TYPE_INCOME) payee = ((Income*) trans)->payer();
				}
				split->addTransaction(trans);
			}
		}
	}
	if(!split) return;
	if(!payee.isEmpty()) split->setPayee(payee);
	if(!mainWin->editSplitTransaction(split, this)) {
		split->clear(true);
		delete split;
	}
}
void LedgerDialog::splitUpTransaction() {
	QList<QTreeWidgetItem*> selection = transactionsView->selectedItems();
	LedgerListViewItem *i = (LedgerListViewItem*) selection.first();
	if(i) {
		if(i->splitTransaction() && i->splitTransaction()->type() == SPLIT_TRANSACTION_TYPE_MULTIPLE_ITEMS) mainWin->splitUpTransaction(i->splitTransaction());
		else if(i->transaction() && i->transaction()->parentSplit() && i->transaction()->parentSplit()->type() == SPLIT_TRANSACTION_TYPE_MULTIPLE_ITEMS) mainWin->splitUpTransaction(i->transaction()->parentSplit());
	}
}
void LedgerDialog::transactionSelectionChanged() {
	QList<QTreeWidgetItem*> selection = transactionsView->selectedItems();
	bool selected = !selection.isEmpty();	
	bool b_join = selected;
	bool b_split = selected;
	bool b_edit = selected;
	bool b_remove = selected;
	SplitTransaction *split = NULL;
	for(int index = 0; index < selection.size(); index++) {
		LedgerListViewItem *i = (LedgerListViewItem*) selection[index];
		Transaction *trans = i->transaction();
		if(i->splitTransaction()) {
			if(b_edit && (index > 0 || selection.size() > 1)) b_edit = false;
			b_join = false;
			if(b_split) b_split = (selection.count() == 1) && (i->splitTransaction()->type() != SPLIT_TRANSACTION_TYPE_LOAN);
		} else if(!trans) {
			if(selection.size() > 1) b_edit = false;
			b_remove = false;
			b_join = false;
			b_split = false;
			break;
		} else {
			if(b_edit && trans->parentSplit() && (index > 0 || selection.size() > 1)) b_edit = false;
			if(b_split) {
				if(!split) split = trans->parentSplit();
				if(!trans->parentSplit() || trans->parentSplit() != split || trans->parentSplit()->type() == SPLIT_TRANSACTION_TYPE_LOAN) b_split = false;
			}
			if(b_join && trans->parentSplit()) b_join = false;
		}
	}
	joinButton->setEnabled(b_join);
	splitUpButton->setEnabled(b_split);
	removeButton->setEnabled(b_remove);
	editButton->setEnabled(b_edit);
}
void LedgerDialog::newExpense() {
	mainWin->newScheduledTransaction(TRANSACTION_TYPE_EXPENSE, NULL, false, this, account);
}
void LedgerDialog::newIncome() {
	mainWin->newScheduledTransaction(TRANSACTION_TYPE_INCOME, NULL, false, this, account);
}
void LedgerDialog::newTransfer() {
	mainWin->newScheduledTransaction(TRANSACTION_TYPE_TRANSFER, NULL, false, this, account);
}
void LedgerDialog::newSplit() {
	mainWin->newMultiItemTransaction(this, account);
}
void LedgerDialog::newDebtPayment() {
	mainWin->newDebtPayment(this, account, false);
}
void LedgerDialog::newDebtInterest() {
	mainWin->newDebtPayment(this, account, true);
}
void LedgerDialog::remove() {
	QList<QTreeWidgetItem*> selection = transactionsView->selectedItems();
	if(selection.count() > 1) {
		if(QMessageBox::warning(this, tr("Delete transactions?"), tr("Are you sure you want to delete all (%1) selected transactions?").arg(selection.count()), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
			return;
		}
	}
	transactionsView->clearSelection();
	for(int index = 0; index < selection.size(); index++) {
		LedgerListViewItem *i = (LedgerListViewItem*) selection[index];
		if(i->splitTransaction()) {
			SplitTransaction *split = i->splitTransaction();
			budget->removeSplitTransaction(split, true);
			mainWin->transactionRemoved(split);
			delete split;
		} else if(i->transaction()) {
			Transaction *trans = i->transaction();
			if(trans->parentSplit() && trans->parentSplit()->count() == 1) {
				SplitTransaction *split = trans->parentSplit();
				budget->removeSplitTransaction(split, true);
				mainWin->transactionRemoved(split);
				delete split;
			} else if(trans->parentSplit() && trans->parentSplit()->type() != SPLIT_TRANSACTION_TYPE_LOAN && trans->parentSplit()->count() == 2) {
				SplitTransaction *split = trans->parentSplit();
				mainWin->splitUpTransaction(split);
				budget->removeTransaction(trans, true);
				mainWin->transactionRemoved(trans);
				delete trans;
			} else {
				budget->removeTransaction(trans, true);
				mainWin->transactionRemoved(trans);
				delete trans;
			}
		}
	}
}
void LedgerDialog::edit() {
	QList<QTreeWidgetItem*> selection = transactionsView->selectedItems();
	if(selection.count() == 1) {
		LedgerListViewItem *i = (LedgerListViewItem*) selection.first();
		if(i->splitTransaction()) mainWin->editSplitTransaction(i->splitTransaction(), this);
		else if(!i->transaction()) mainWin->editAccount(account, this);
		else if(i->transaction()->parentSplit()) mainWin->editSplitTransaction(i->transaction()->parentSplit(), this);
		else if(i->transaction()) mainWin->editTransaction(i->transaction(), this);
	} else if(selection.count() > 1) {
		bool warned1 = false, warned2 = false, warned3 = false;
		bool equal_date = true, equal_description = true, equal_value = true, equal_category = true, equal_payee = b_extra;
		int transtype = -1;
		Transaction *comptrans = NULL;
		Account *compcat = NULL;
		QDate compdate;
		for(int index = 0; index < selection.size(); index++) {
			LedgerListViewItem *i = (LedgerListViewItem*) selection[index];
			if(i->transaction() && !i->transaction()->parentSplit()) {
				if(!comptrans) {
					comptrans = i->transaction();
					compdate = i->transaction()->date();
					transtype = i->transaction()->type();
					if(i->transaction()->type() != TRANSACTION_TYPE_EXPENSE && i->transaction()->type() != TRANSACTION_TYPE_INCOME) equal_payee = false;
					if(i->transaction()->type() == TRANSACTION_TYPE_INCOME) {
						compcat = ((Income*) i->transaction())->category();
					} else if(i->transaction()->type() == TRANSACTION_TYPE_EXPENSE) {
						compcat = ((Expense*) i->transaction())->category();
					} else if(i->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY || i->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL) {
						equal_value = false;
						equal_description = false;
						compcat = ((SecurityTransaction*) i->transaction())->account();
						if(compcat->type() == ACCOUNT_TYPE_ASSETS) {
							equal_category = false;
						}
					}
				} else {
					if(transtype >= 0 && comptrans->type() != transtype) {
						transtype = -1;
					}
					if(equal_date && (compdate != i->transaction()->date())) {
						equal_date = false;
					}
					if(equal_description && (i->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY || i->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL || comptrans->description() != i->transaction()->description())) {
						equal_description = false;
					}
					if(equal_payee && (i->transaction()->type() != comptrans->type() || (comptrans->type() == TRANSACTION_TYPE_EXPENSE && ((Expense*) comptrans)->payee() != ((Expense*) i->transaction())->payee()) || (comptrans->type() == TRANSACTION_TYPE_INCOME && ((Income*) comptrans)->payer() != ((Income*) i->transaction())->payer()))) {
						equal_payee = false;
					}
					if(equal_value && (i->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY || i->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL || comptrans->value() != i->transaction()->value())) {
						equal_value = false;
					}
					if(equal_category) {
						if(i->transaction()->type() == TRANSACTION_TYPE_INCOME) {
							if(compcat != ((Income*) i->transaction())->category()) {
								equal_category = false;
							}
						} else if(i->transaction()->type() == TRANSACTION_TYPE_EXPENSE) {
							if(compcat != ((Expense*) i->transaction())->category()) {
								equal_category = false;
							}
						} else if(i->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY || i->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL) {
							if(compcat != ((SecurityTransaction*) i->transaction())->account()) {
								equal_category = false;
							}
						}
					}
				}
			}
		}
		MultipleTransactionsEditDialog *dialog = new MultipleTransactionsEditDialog(b_extra, transtype, budget, this);
		LedgerListViewItem *i = (LedgerListViewItem*) transactionsView->currentItem();
		if(!i->isSelected()) i = (LedgerListViewItem*) selection.first();
		if(i) {
			dialog->setTransaction(i->transaction());
		}
		if(equal_description) dialog->descriptionButton->setChecked(true);
		if(equal_payee && dialog->payeeButton) dialog->payeeButton->setChecked(true);
		if(equal_value && dialog->valueButton) dialog->valueButton->setChecked(true);
		if(equal_date) dialog->dateButton->setChecked(true);
		if(equal_category && dialog->categoryButton) dialog->categoryButton->setChecked(true);
		if(dialog->exec() == QDialog::Accepted) {
			QDate date = dialog->date();
			bool future = !date.isNull() && date > QDate::currentDate();
			for(int index = 0; index < selection.size(); index++) {
				LedgerListViewItem *i = (LedgerListViewItem*) selection[index];
				if(i->transaction() && !i->transaction()->parentSplit()) {
					if(!warned1 && (i->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY || i->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL)) {
						if(dialog->valueButton && dialog->valueButton->isChecked()) {
							QMessageBox::critical(this, tr("Error"), tr("Cannot set the value of security transactions using the dialog for modifying multiple transactions.", "Financial security (e.g. stock, mutual fund)"));
							warned1 = true;
						}
					}
					if(!warned2 && (i->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY || i->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL || (i->transaction()->type() == TRANSACTION_TYPE_INCOME && ((Income*) i->transaction())->security()))) {
						if(dialog->descriptionButton->isChecked()) {
							QMessageBox::critical(this, tr("Error"), tr("Cannot change description of dividends and security transactions.", "Referring to the transaction description property (transaction title/generic article name); Financial security (e.g. stock, mutual fund)"));
							warned2 = true;
						}
					}
					if(!warned3 && dialog->payeeButton && (i->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY || i->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL || (i->transaction()->type() == TRANSACTION_TYPE_INCOME && ((Income*) i->transaction())->security()))) {
						if(dialog->payeeButton->isChecked()) {
							QMessageBox::critical(this, tr("Error"), tr("Cannot change payer of dividends and security transactions.", "Financial security (e.g. stock, mutual fund)"));
							warned3 = true;
						}
					}
					Transaction *trans = i->transaction();
					Transaction *oldtrans = trans->copy();
					if(dialog->modifyTransaction(trans)) {
						if(future) {
							budget->removeTransaction(trans, true);
							mainWin->transactionRemoved(trans);
							ScheduledTransaction *strans = new ScheduledTransaction(budget, trans, NULL);
							budget->addScheduledTransaction(strans);
							mainWin->transactionAdded(strans);
						} else {
							mainWin->transactionModified(trans, oldtrans);
						}
					}
					delete oldtrans;
				}
			}
		}
		dialog->deleteLater();
	}
}
void LedgerDialog::edit(QTreeWidgetItem*) {
	edit();
}
void LedgerDialog::updateTransactions() {
	int scroll_h = transactionsView->horizontalScrollBar()->value();
	int scroll_v = transactionsView->verticalScrollBar()->value();
	Transaction *selected_transaction = NULL;
	SplitTransaction *selected_split = NULL;
	QList<QTreeWidgetItem*> selection = transactionsView->selectedItems();
	if(selection.count() == 1) {
		LedgerListViewItem *i = (LedgerListViewItem*) selection.first();
		selected_split = i->splitTransaction();
		if(!selected_split) selected_transaction = i->transaction();
	}
	transactionsView->clear();
	double balance = account->initialBalance();
	double total_balance = account->initialBalance();
	double reductions = 0.0;
	double expenses = 0.0;
	int quantity = 0;
	QDate curdate = QDate::currentDate();
	if(balance != 0.0) new LedgerListViewItem(NULL, NULL, transactionsView, "-", "-", tr("Opening balance", "Account balance"), "-", QString::null, QString::null, QLocale().toCurrencyString(balance));
	Transaction *trans = budget->transactions.first();
	SplitTransaction *split = budget->splitTransactions.first();
	Transactions *transs = trans;
	if(!transs || (split && split->date() < trans->date())) transs = split;
	QVector<SplitTransaction*> splits;
	while(transs) {
		if(transs == split) {
			if(split->type() == SPLIT_TRANSACTION_TYPE_MULTIPLE_ITEMS && ((MultiItemTransaction*) split)->account() == account) {
				double value = split->accountChange(account);
				balance += value;
				total_balance += balance;
				LedgerListViewItem *i = new LedgerListViewItem(NULL, split, NULL, QLocale().toString(split->date(), QLocale::ShortFormat), tr("Split Transaction"), split->description(), ((MultiItemTransaction*) split)->fromAccountsString(), (value >= 0.0) ? QLocale().toCurrencyString(value) : QString::null, (value < 0.0) ? QLocale().toCurrencyString(-value) : QString::null, QLocale().toCurrencyString(balance));
				transactionsView->insertTopLevelItem(0, i);
				i->setColors();
				quantity++;
				if(split == selected_split) {
					i->setSelected(true);
				}
			} else if(split->type() == SPLIT_TRANSACTION_TYPE_LOAN) {
				if(((DebtPayment*) split)->loan() == account) {
					DebtPayment *lsplit = (DebtPayment*) split;
					Transaction *ltrans = lsplit->paymentTransaction();
					if(ltrans) {
						double value = ltrans->value();
						balance += value;
						total_balance += balance;
						reductions += value;
						LedgerListViewItem *i = new LedgerListViewItem(ltrans, NULL, NULL, QLocale().toString(split->date(), QLocale::ShortFormat), tr("Debt Payment"), tr("Reduction"), ltrans->fromAccount()->name(), (value >= 0.0) ? QLocale().toCurrencyString(value) : QString::null, value >= 0.0 ? QString::null : QLocale().toCurrencyString(-value), QLocale().toCurrencyString(balance));
						transactionsView->insertTopLevelItem(0, i);
						i->setColors();
						if(ltrans == selected_transaction) {
							i->setSelected(true);
						}
						quantity++;
					}
					ltrans = lsplit->feeTransaction();
					if(ltrans) {
						double value = ltrans->value();
						expenses += value;
						bool to_balance = (ltrans->fromAccount() == account);
						if(to_balance) {
							balance -= value;
							total_balance += balance;
							reductions -= value;
							quantity++;
						}
						LedgerListViewItem *i = new LedgerListViewItem(ltrans, NULL, NULL, QLocale().toString(split->date(), QLocale::ShortFormat), tr("Debt Payment"), tr("Fee"), ltrans->fromAccount()->name(), (to_balance || value >= 0.0) ? QString::null : QLocale().toCurrencyString(-value), (to_balance && value >= 0.0) ? QLocale().toCurrencyString(value) : QString::null, QLocale().toCurrencyString(balance), to_balance ? QString::null : QLocale().toCurrencyString(value));
						transactionsView->insertTopLevelItem(0, i);
						i->setColors();
						if(ltrans == selected_transaction) {
							i->setSelected(true);
						}
					}
					ltrans = lsplit->interestTransaction();
					if(ltrans) {
						double value = ltrans->value();
						expenses += value;
						bool to_balance = (ltrans->fromAccount() == account);
						if(to_balance) {
							balance -= value;
							total_balance += balance;
							reductions -= value;
							quantity++;
						}
						LedgerListViewItem *i = new LedgerListViewItem(ltrans, NULL, NULL, QLocale().toString(split->date(), QLocale::ShortFormat), tr("Debt Payment"), tr("Interest"), ltrans->fromAccount()->name(), (to_balance || value >= 0.0) ? QString::null : QLocale().toCurrencyString(-value), (to_balance && value >= 0.0) ? QLocale().toCurrencyString(value) : QString::null, QLocale().toCurrencyString(balance), to_balance ? QString::null : QLocale().toCurrencyString(value));
						transactionsView->insertTopLevelItem(0, i);
						i->setColors();
						if(ltrans == selected_transaction) {
							i->setSelected(true);
						}
					}
				} else if(((DebtPayment*) split)->account() == account) {
					double value = split->accountChange(account);
					if(value != 0.0) {
						balance += value;
						total_balance += balance;
						LedgerListViewItem *i = new LedgerListViewItem(NULL, split, NULL, QLocale().toString(split->date(), QLocale::ShortFormat), tr("Debt Payment"), split->description(), ((DebtPayment*) split)->loan()->name(), (value >= 0.0) ? QLocale().toCurrencyString(value) : QString::null, (value < 0.0) ? QLocale().toCurrencyString(-value) : QString::null, QLocale().toCurrencyString(balance));
						transactionsView->insertTopLevelItem(0, i);
						i->setColors();
						quantity++;
						if(split == selected_split) {
							i->setSelected(true);
						}
					}
				}
			}
		} else if(transs->relatesToAccount(account) && (!trans->parentSplit() || trans->parentSplit()->type() == SPLIT_TRANSACTION_TYPE_MULTIPLE_ACCOUNTS || (trans->parentSplit()->type() == SPLIT_TRANSACTION_TYPE_MULTIPLE_ITEMS && ((MultiItemTransaction*) trans->parentSplit())->account() != account))) {
			double value = trans->accountChange(account);
			balance += value;
			total_balance += balance;
			LedgerListViewItem *i = new LedgerListViewItem(trans, NULL, NULL, QLocale().toString(trans->date(), QLocale::ShortFormat), QString::null, trans->description(), account == trans->fromAccount() ? trans->toAccount()->name() : trans->fromAccount()->name(), (value >= 0.0) ? QLocale().toCurrencyString(value) : QString::null, value >= 0.0 ? QString::null : QLocale().toCurrencyString(-value), QLocale().toCurrencyString(balance));
			quantity++;
			transactionsView->insertTopLevelItem(0, i);
			i->setColors();
			if(trans->type() == TRANSACTION_TYPE_INCOME) {
				if(value >= 0.0) i->setText(1, tr("Income"));
				else i->setText(1, tr("Repayment"));
			} else if(trans->type() == TRANSACTION_TYPE_EXPENSE) {
				if(value <= 0.0) i->setText(1, tr("Expense"));
				else i->setText(1, tr("Refund"));
			} else if(trans->relatesToAccount(budget->balancingAccount)) {
				 i->setText(1, tr("Account Balance Adjustment"));
			} else {
				i->setText(1, tr("Transfer"));
			}
			if(trans == selected_transaction) {
				i->setSelected(true);
			}
		}
		if(transs == trans) {
			trans = budget->transactions.next();
			if(trans && trans->date() > curdate) trans = NULL;
		} else {
			split = budget->splitTransactions.next();
			if(split && split->date() > curdate) split = NULL;
		}
		transs = trans;
		if(!transs || (split && split->date() < trans->date())) transs = split;
	}
	if(account->accountType() == ASSETS_TYPE_LIABILITIES || account->accountType() == ASSETS_TYPE_CREDIT_CARD) {
		statLabel->setText(QString("<div align=\"right\"><b>%1</b> %4 &nbsp; <b>%2</b> %5 &nbsp; <b>%3</b> %6</div>").arg(tr("Current debt:")).arg(tr("Total debt reduction:")).arg(tr("Total interest and fees:")).arg(QLocale().toCurrencyString(-balance)).arg(QLocale().toCurrencyString(reductions)).arg(QLocale().toCurrencyString(expenses)));
	} else {	
		statLabel->setText(QString("<div align=\"right\"><b>%1</b> %4 &nbsp; <b>%2</b> %5 &nbsp; <b>%3</b> %6</div>").arg(tr("Current balance:", "Account balance")).arg(tr("Average balance:", "Account balance")).arg(tr("Number of transactions:")).arg(QLocale().toCurrencyString(balance)).arg(QLocale().toCurrencyString(total_balance / (quantity + 1))).arg(QLocale().toString(quantity)));
	}
	transactionsView->horizontalScrollBar()->setValue(scroll_h);
	transactionsView->verticalScrollBar()->setValue(scroll_v);
}
void LedgerDialog::reject() {
	saveConfig();
	QDialog::reject();
}

