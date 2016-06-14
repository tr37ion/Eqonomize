/***************************************************************************
 *   Copyright (C) 2006-2008, 2014, 2016 by Hanna Knutsson                 *
 *   hanna_k@fmgirl.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef EQONOMIZE_H
#define EQONOMIZE_H

#include <QDateTime>
#include <QDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMap>
#include <QModelIndex>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVector>
#include <QWizardPage>
#include <QUrl>
#include <QApplication>

#include <kxmlguiwindow.h>

class QAction;
class QCheckBox;
class QLabel;
class QMenu;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTabWidget;
class QVBoxLayout;
class QComboBox;
class QAction;
class QLineEdit;
class QCommandLineParser;

class KAutoSaveFile;
class QDateEdit;
class KMenu;
class KPageWidget;
class KPageWidgetItem;
class KRecentFilesAction;
class KSelectAction;
class QTextEdit;
class KToggleAction;
class KToolBar;

class CategoriesComparisonChart;
class CategoriesComparisonReport;
class OverTimeChart;
class OverTimeReport;
class Account;
class AssetsAccount;
class Budget;
class ConfirmScheduleListViewItem;
class EqonomizeMonthSelector;
class EqonomizeValueEdit;
class Expense;
class ExpensesAccount;
class Income;
class IncomesAccount;
class ReinvestedDividend;
class ScheduledTransaction;
class Security;
class SecurityTrade;
class SplitTransaction;
class Transaction;
class TransactionListWidget;
class Transfer;

typedef enum {
	INITIAL_PERIOD_CURRENT_MONTH,
	INITIAL_PERIOD_CURRENT_YEAR,
	INITIAL_PERIOD_CURRENT_WHOLE_MONTH,
	INITIAL_PERIOD_CURRENT_WHOLE_YEAR,
	INITIAL_PERIOD_LAST
} InitialPeriod;

class Eqonomize : public KXmlGuiWindow {
	
	Q_OBJECT

	public:

		Eqonomize();
		virtual ~Eqonomize();
		
		bool saveURL(const QUrl& url);
		bool askSave(bool before_exit = false);
		void createDefaultBudget();
		void readFileDependentOptions();

		Budget *budget;

		bool first_run;

		void appendFilterExpense(Expense *expense, bool update_total_cost, bool update_accounts);
		void appendFilterIncome(Income *income, bool update_total_income, bool update_accounts);
		void appendFilterTransfer(Transfer *transfer, bool update_total_amount, bool update_accounts);
		bool filterTransaction(Transaction *trans);
		void subtractScheduledTransactionValue(ScheduledTransaction *strans, bool update_value_display);
		void addScheduledTransactionValue(ScheduledTransaction *strans, bool update_value_display, bool subtract = false);
		void subtractTransactionValue(Transaction *trans, bool update_value_display);
		void addTransactionValue(Transaction *trans, const QDate &transdate, bool update_value_display, bool subtract = false, int n = -1, int b_future = -1, const QDate *monthdate = NULL);
		void appendIncomesAccount(IncomesAccount *account);
		void appendExpensesAccount(ExpensesAccount *account);
		void appendAssetsAccount(AssetsAccount *account);
		void updateMonthlyBudget(Account *account);
		void updateTotalMonthlyExpensesBudget();
		void updateTotalMonthlyIncomesBudget();
		bool editAccount(Account*);
		bool editAccount(Account*, QWidget *parent);
		void balanceAccount(Account*);
		bool checkSchedule(bool update_display);
		void updateScheduledTransactions();
		void appendScheduledTransaction(ScheduledTransaction *strans);
		bool editScheduledTransaction(ScheduledTransaction *strans);
		bool editScheduledTransaction(ScheduledTransaction *strans, QWidget *parent);
		bool editOccurrence(ScheduledTransaction *strans, const QDate &date);
		bool editOccurrence(ScheduledTransaction *strans, const QDate &date, QWidget *parent);
		bool editTransaction(Transaction *trans, QWidget *parent);
		bool editTransaction(Transaction *trans);
		bool removeScheduledTransaction(ScheduledTransaction *strans);
		bool removeOccurrence(ScheduledTransaction *strans, const QDate &date);
		bool newScheduledTransaction(int transaction_type, Security *security = NULL, bool select_security = false);
		bool newScheduledTransaction(int transaction_type, Security *security, bool select_security, QWidget *parent, Account *account = NULL);
		bool newSplitTransaction(QWidget *parent, AssetsAccount *account = NULL);
		bool editSplitTransaction(SplitTransaction *split);
		bool editSplitTransaction(SplitTransaction *split, QWidget *parent);
		bool splitUpTransaction(SplitTransaction *split);
		bool removeSplitTransaction(SplitTransaction *split);
		bool saveView(QTextStream &file, int fileformat);
		bool exportScheduleList(QTextStream &outf, int fileformat);
		bool exportAccountsList(QTextStream &outf, int fileformat);
		bool exportSecuritiesList(QTextStream &outf, int fileformat);
		void editSecurity(QTreeWidgetItem *i);
		void appendSecurity(Security *security);
		void updateSecurity(Security *security);
		void updateSecurity(QTreeWidgetItem *i);
		void updateSecurityAccount(AssetsAccount *account, bool update_display = true);
		bool editReinvestedDividend(ReinvestedDividend *rediv, Security *security, QWidget *parent);
		void editReinvestedDividend(ReinvestedDividend *rediv, Security *security);
		bool editSecurityTrade(SecurityTrade *ts, QWidget *parent);
		void editSecurityTrade(SecurityTrade *ts);
		void setModified(bool has_been_modified = true);
		void showExpenses();
		void showIncomes();
		void showTransfers();
		void updateSecuritiesStatistics();
		bool crashRecovery(QUrl url);
		bool newRefundRepayment(Transaction *trans);
		void readOptions();
		void setCommandLineParser(QCommandLineParser*);

		QAction *ActionAP_1, *ActionAP_2, *ActionAP_3, *ActionAP_4, *ActionAP_5, *ActionAP_6, *ActionAP_7, *ActionAP_8;
		QAction *ActionEditSchedule, *ActionEditOccurrence, *ActionDeleteSchedule, *ActionDeleteOccurrence;
		QAction *ActionAddAccount, *ActionNewAssetsAccount, *ActionNewIncomesAccount, *ActionNewExpensesAccount, *ActionEditAccount, *ActionDeleteAccount, *ActionBalanceAccount;
		QAction *ActionShowAccountTransactions;
		QAction *ActionNewExpense, *ActionNewIncome, *ActionNewTransfer, *ActionNewSplitTransaction;
		QAction *ActionEditTransaction, *ActionEditScheduledTransaction, *ActionEditSplitTransaction;
		QAction *ActionJoinTransactions, *ActionSplitUpTransaction;
		QAction *ActionDeleteTransaction, *ActionDeleteScheduledTransaction, *ActionDeleteSplitTransaction;
		QAction *ActionNewSecurity, *ActionEditSecurity, *ActionBuyShares, *ActionSellShares, *ActionNewDividend, *ActionNewReinvestedDividend, *ActionNewSecurityTrade, *ActionSetQuotation, *ActionEditQuotations, *ActionEditSecurityTransactions, *ActionDeleteSecurity;
		QAction *ActionNewRefund, *ActionNewRepayment, *ActionNewRefundRepayment;
		QAction *ActionSave, *ActionFileReload, *ActionSaveView, *ActionPrintView;
		QAction *ActionOverTimeReport, *ActionCategoriesComparisonReport, *ActionOverTimeChart, *ActionCategoriesComparisonChart;
		QAction *ActionImportCSV, *ActionImportQIF, *ActionExportQIF;
		QAction *ActionExtraProperties;
		KSelectAction *ActionSelectInitialPeriod;
		KRecentFilesAction *ActionOpenRecent;
		
	protected:

		void setupActions();
		void saveOptions();
		bool queryClose();
		
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);

		QUrl current_url;
		double period_months, from_to_months;
		bool modified, modified_auto_save, auto_save_timeout;
		QDate from_date, to_date, frommonth_begin, prevmonth_begin;
		QDate securities_from_date, securities_to_date;
		QDate prev_cur_date;
		bool partial_budget;
		bool b_extra;
		KAutoSaveFile *cr_tmp_file;

		KToolBar *scheduleButtons, *securitiesButtons;
		KPageWidget *tabs;
		QTabWidget *accountsTabs;
		QCheckBox *budgetButton;
		EqonomizeValueEdit *budgetEdit;
		EqonomizeMonthSelector *budgetMonthEdit;
		QLabel *prevMonthBudgetLabel;
		QWidget *accounts_page, *expenses_page, *incomes_page, *transfers_page, *securities_page, *schedule_page;
		KPageWidgetItem *accounts_page_item, *expenses_page_item, *incomes_page_item, *transfers_page_item, *securities_page_item, *schedule_page_item;
		QVBoxLayout *expensesLayout, *incomesLayout, *transfersLayout;
		TransactionListWidget *expensesWidget, *incomesWidget, *transfersWidget;
		QTreeWidget *accountsView, *securitiesView, *scheduleView;
		QTreeWidgetItem *assetsItem, *incomesItem, *expensesItem;
		QCheckBox *accountsPeriodFromButton;
		QDateEdit *accountsPeriodFromEdit, *accountsPeriodToEdit;
		QCheckBox *partialBudgetButton;
		QComboBox *accountsPeriodCombo;
		QCheckBox *securitiesPeriodFromButton;
		QDateEdit *securitiesPeriodFromEdit, *securitiesPeriodToEdit;
		QPushButton *newScheduleButton, *editScheduleButton, *removeScheduleButton;
		QMenu *editScheduleMenu, *removeScheduleMenu;
		QPushButton *newSecurityTransactionButton, *newSecurityButton, *setQuotationButton;
		QLabel *securitiesStatLabel;
		QLabel *footer1;
		QCommandLineParser *parser;

		double total_value, total_cost, total_profit, total_rate;
		double expenses_accounts_value, incomes_accounts_value, assets_accounts_value;
		double expenses_accounts_change, incomes_accounts_change, assets_accounts_change;
		double expenses_budget, expenses_budget_diff, incomes_budget, incomes_budget_diff;
		QMap<Account*, double> account_value;
		QMap<Account*, double> account_change;
		QMap<Account*, QMap<QDate, double> > account_month;
		QMap<Account*, double> account_month_begincur;
		QMap<Account*, double> account_month_beginfirst;
		QMap<Account*, double> account_month_endlast;
		QMap<Account*, double> account_budget;
		QMap<Account*, double> account_budget_diff;
		QMap<Account*, double> account_future_diff;
		QMap<Account*, double> account_future_diff_change;
		QMap<QTreeWidgetItem*, Account*> account_items;
		QMap<Account*, QTreeWidgetItem*> item_accounts;

		QMenu *accountPopupMenu, *securitiesPopupMenu, *schedulePopupMenu;

	public slots:

		void saveCrashRecovery();
		void autoSave();
		void onAutoSaveTimeout();
		
		void onActivateRequested(const QStringList&, const QString&);

		void useExtraProperties(bool);
	
		void importCSV();
		void importQIF();
		void exportQIF();

		void reloadBudget();

		void showOverTimeReport();
		void showCategoriesComparisonReport();
		void showOverTimeChart();
		void showCategoriesComparisonChart();
		void printView();
		void saveView();

		void newSecurity();
		void editSecurity();
		void deleteSecurity();
		void buySecurities();
		void sellSecurities();
		void newDividend();
		void newReinvestedDividend();
		void newSecurityTrade();
		void setQuotation();
		void editQuotations();
		void editSecurityTransactions();
		void securitiesSelectionChanged();
		void securitiesExecuted(QTreeWidgetItem*);
		void securitiesExecuted(QTreeWidgetItem*, int);
		void popupSecuritiesMenu(const QPoint&);
		void updateSecurities();
		
		void newSplitTransaction();
		void newScheduledExpense();
		void newScheduledIncome();
		void newScheduledTransfer();
		void editScheduledTransaction();
		void editOccurrence();
		void removeScheduledTransaction();
		void removeOccurrence();
		void scheduleSelectionChanged();
		void scheduleExecuted(QTreeWidgetItem*);
		void popupScheduleMenu(const QPoint&);

		void editSelectedScheduledTransaction();
		void editSelectedTransaction();
		void editSelectedSplitTransaction();
		void deleteSelectedScheduledTransaction();
		void deleteSelectedTransaction();
		void deleteSelectedSplitTransaction();
		void joinSelectedTransactions();
		void splitUpSelectedTransaction();

		void newRefund();
		void newRepayment();
		void newRefundRepayment();

		void onPageChange(KPageWidgetItem*, KPageWidgetItem*);
		void onPageChange(QWidget*);

		void showAccountTransactions(bool = false);

		void updateTransactionActions();
		
		void openURL(const QUrl&);
		void fileNew();
		void fileOpen();
		void fileOpenRecent(const QUrl&);
		void fileReload();
		void fileSave();
		void fileSaveAs();
		void optionsPreferences();

		void checkSchedule();

		void checkDate();

		void popupAccountsMenu(const QPoint&);

		void addAccount();
		void newAssetsAccount();
		void newIncomesAccount();
		void newExpensesAccount();
		void accountExecuted(QTreeWidgetItem*, int);
		void accountExecuted(QTreeWidgetItem*);
		void balanceAccount();
		void editAccount();
		void deleteAccount();
		void accountsSelectionChanged();

		void setPartialBudget(bool);

		void budgetEditReturnPressed();
		void budgetMonthChanged(const QDate&);
		void budgetChanged(double);
		void budgetToggled(bool);
		void updateBudgetEdit();
		
		void accountsPeriodFromChanged(const QDate&);
		void accountsPeriodToChanged(const QDate&);
		void periodSelected(QAction*);
		void prevMonth();
		void nextMonth();
		void currentMonth();
		void prevYear();
		void nextYear();
		void currentYear();

		void securitiesPeriodFromChanged(const QDate&);
		void securitiesPeriodToChanged(const QDate&);
		void securitiesPrevMonth();
		void securitiesNextMonth();
		void securitiesCurrentMonth();
		void securitiesPrevYear();
		void securitiesNextYear();
		void securitiesCurrentYear();

		void transactionAdded(Transaction*);
		void transactionModified(Transaction*, Transaction*);
		void transactionRemoved(Transaction*);

		void scheduledTransactionAdded(ScheduledTransaction*);
		void scheduledTransactionModified(ScheduledTransaction*, ScheduledTransaction*);
		void scheduledTransactionRemoved(ScheduledTransaction*);
		void scheduledTransactionRemoved(ScheduledTransaction*, ScheduledTransaction*);

		void splitTransactionAdded(SplitTransaction*);
		void splitTransactionRemoved(SplitTransaction*);

		void filterAccounts();

	signals:

		void accountsModified();
		void transactionsModified();
		void budgetUpdated();
		void timeToSaveConfig();

};

class OverTimeReportDialog : public QDialog {
	
	Q_OBJECT
	
	public:
		
		OverTimeReportDialog(Budget *budg, QWidget *parent);
		OverTimeReport *report;

	public slots:
		
		void reject();
		
};

class CategoriesComparisonReportDialog : public QDialog {
	
	Q_OBJECT
	
	public:
		
		CategoriesComparisonReportDialog(bool extra_parameters, Budget *budg, QWidget *parent);
		CategoriesComparisonReport *report;

	public slots:
		
		void reject();
		
};

class CategoriesComparisonChartDialog : public QDialog {
	
	Q_OBJECT
		
	public:
		
		CategoriesComparisonChartDialog(Budget *budg, QWidget *parent);		
		CategoriesComparisonChart *chart;

	public slots:
		
		void reject();
		
};

class OverTimeChartDialog : public QDialog {
	
	Q_OBJECT
		
	public:
		
		OverTimeChartDialog(bool extra_parameters, Budget *budg, QWidget *parent);		
		OverTimeChart *chart;

	public slots:
		
		void reject();
		
};

class ConfirmScheduleDialog : public QDialog {
	
	Q_OBJECT
	
	protected:

		QTreeWidget *transactionsView;
		Budget *budget;
		bool b_extra;
		QPushButton *editButton, *removeButton, *postponeButton;
		ConfirmScheduleListViewItem *current_item;
		int current_index;
		
	public:
		
		ConfirmScheduleDialog(bool extra_parameters, Budget *budg, QWidget *parent, QString title);

		Transaction *firstTransaction();
		Transaction *nextTransaction();

	public slots:
		
		void transactionSelectionChanged();
		void remove();
		void edit();
		void postpone();
		void updateTransactions();
		
};

class EditAssetsAccountDialog : public QDialog {

	Q_OBJECT
	
	protected:

		QLineEdit *nameEdit;
		EqonomizeValueEdit *valueEdit;
		QTextEdit *descriptionEdit;
		QComboBox *typeCombo;
		QCheckBox *budgetButton;
		Budget *budget;
		Account *current_account;
		
	public:
		
		EditAssetsAccountDialog(Budget *budg, QWidget *parent, QString title);

		AssetsAccount *newAccount();
		void modifyAccount(AssetsAccount *account);
		void setAccount(AssetsAccount *account);

	protected slots:

		void typeActivated(int);
		void accept();
		
};

class EditExpensesAccountDialog : public QDialog {

	Q_OBJECT
	
	protected:

		QLineEdit *nameEdit;
		EqonomizeValueEdit *budgetEdit;
		QTextEdit *descriptionEdit;
		QCheckBox *budgetButton;
		Budget *budget;
		Account *current_account;
		
	public:
		
		EditExpensesAccountDialog(Budget *budg, QWidget *parent, QString title);
		ExpensesAccount *newAccount();
		void modifyAccount(ExpensesAccount *account);
		void setAccount(ExpensesAccount *account);

	protected slots:

		void budgetEnabled(bool);
		void accept();
		
};

class EditIncomesAccountDialog : public QDialog {

	Q_OBJECT
	
	protected:

		QLineEdit *nameEdit;
		EqonomizeValueEdit *budgetEdit;
		QTextEdit *descriptionEdit;
		QCheckBox *budgetButton;
		Budget *budget;
		Account *current_account;
		
	public:
		
		EditIncomesAccountDialog(Budget *budg, QWidget *parent, QString title);
		IncomesAccount *newAccount();
		void modifyAccount(IncomesAccount *account);
		void setAccount(IncomesAccount *account);

	protected slots:

		void budgetEnabled(bool);
		void accept();
		
};

class EditSecurityDialog : public QDialog {

	Q_OBJECT

	protected:

		QLineEdit *nameEdit;
		QTextEdit *descriptionEdit;
		EqonomizeValueEdit *sharesEdit, *quotationEdit;
		QDateEdit *quotationDateEdit;
		QComboBox *typeCombo, *accountCombo;
		QSpinBox *decimalsEdit;
		QLabel *quotationLabel, *quotationDateLabel;
		Budget *budget;
		QVector<AssetsAccount*> accounts;

	public:
		
		EditSecurityDialog(Budget *budg, QWidget *parent, QString title);
		Security *newSecurity();
		bool modifySecurity(Security *security);
		void setSecurity(Security *security);
		bool checkAccount();

	protected slots:

		void decimalsChanged(int);

};
 
class EditQuotationsDialog : public QDialog {

	Q_OBJECT

	protected:

		QLabel *titleLabel;
		QTreeWidget *quotationsView;
		EqonomizeValueEdit *quotationEdit;
		QDateEdit *dateEdit;
		QPushButton *changeButton, *addButton, *deleteButton;

	public:

		EditQuotationsDialog(QWidget *parent);

		void setSecurity(Security *security);
		void modifyQuotations(Security *security);

	protected slots:
		
		void onSelectionChanged();
		void addQuotation();
		void changeQuotation();
		void deleteQuotation();

};

class RefundDialog : public QDialog {

	Q_OBJECT

	protected:

		Transaction *transaction;
		EqonomizeValueEdit *valueEdit, *quantityEdit;
		QDateEdit *dateEdit;
		QComboBox *accountCombo;
		QLineEdit *commentsEdit;

	public:

		RefundDialog(Transaction *trans, QWidget *parent);

		Transaction *createRefund();
		bool validValues();

	protected slots:
		
		void accept();

};

class EditReinvestedDividendDialog : public QDialog {

	Q_OBJECT

	protected:

		Budget *budget;
		QComboBox *securityCombo;
		EqonomizeValueEdit *sharesEdit;
		QDateEdit *dateEdit;

	public:

		EditReinvestedDividendDialog(Budget *budg, Security *sec, bool select_security, QWidget *parent);

		Security *selectedSecurity();
		void securityChanged();
		void setDividend(ReinvestedDividend *rediv);
		bool modifyDividend(ReinvestedDividend *rediv);
		ReinvestedDividend *createDividend();
		bool validValues();

	protected slots:
		
		void accept();

};

class EditSecurityTradeDialog : public QDialog {

	Q_OBJECT

	protected:

		Budget *budget;
		QComboBox *fromSecurityCombo, *toSecurityCombo;
		EqonomizeValueEdit *valueEdit, *fromSharesEdit, *toSharesEdit;
		QDateEdit *dateEdit;

	public:

		EditSecurityTradeDialog(Budget *budg, Security *sec, QWidget *parent);

		void setSecurityTrade(SecurityTrade *ts);
		SecurityTrade *createSecurityTrade();
		bool validValues();
		bool checkSecurities();

	protected slots:
		
		void accept();
		void maxShares();
		Security *selectedFromSecurity();
		Security *selectedToSecurity();
		void fromSecurityChanged();
		void toSecurityChanged();

};

class SecurityTransactionsDialog : public QDialog {
	
	Q_OBJECT
	
	protected:

		Security *security;
		Eqonomize *mainWin;
		QTreeWidget *transactionsView;
		QPushButton *editButton, *removeButton;

		void updateTransactions();
		
	public:
		
		SecurityTransactionsDialog(Security *sec, Eqonomize *parent, QString title);

	protected slots:
		
		void remove();
		void edit();
		void edit(QTreeWidgetItem*);
		void transactionSelectionChanged();
		
};

class EqonomizeTreeWidget : public QTreeWidget {

	Q_OBJECT
	
	public:
	
		EqonomizeTreeWidget(QWidget *parent);
		EqonomizeTreeWidget();

	protected slots:
	
		void keyPressEvent(QKeyEvent*);
		void onDoubleClick(const QModelIndex&);

	signals:
	
		void returnPressed(QTreeWidgetItem*);
		void doubleClicked(QTreeWidgetItem*, int);
		 
};

class QIFWizardPage : public QWizardPage {
	protected:
		bool is_complete;
	public:		
		QIFWizardPage();
		bool isComplete() const;
		void setComplete(bool b);
};

#endif
