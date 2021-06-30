/* Copyright (c) 2020-2021 AVME Developers
   Distributed under the MIT/X11 software license, see the accompanying
   file LICENSE or http://www.opensource.org/licenses/mit-license.php. */
import QtQuick 2.9
import QtQuick.Controls 2.2

import "qrc:/qml/components"
import "qrc:/qml/panels"

// Screen for listing Accounts and their general operations
// TODO: back button that closes the Wallet

Item {
  id: accountsScreen

  Component.onCompleted: {
    fetchAccounts()
  }

  function fetchAccounts() {
    accountSelectPanel.accountModel.clear()
    var accList = QmlSystem.listAccounts()
    for (var i = 0; i < accList.length; i++) {
      accountSelectPanel.accountModel.set(i, JSON.parse(accList[i]))
    }
    fetchBalances()
  }

  function fetchBalances() {
    for (var i = 0; i < accountSelectPanel.accountModel.count; i++) {
      var address = accountSelectPanel.accountModel.get(i).address
      var bal = QmlSystem.getAccountAVAXBalance(address)
      var usd = QmlSystem.getAccountAVAXValue(address, bal)
      accountSelectPanel.accountModel.setProperty(i, "coinAmount", bal + " AVAX")
      accountSelectPanel.accountModel.setProperty(i, "coinValue", "$" + usd)
    }
  }

  AVMEPanelAccountSelect {
    id: accountSelectPanel
    height: parent.height * 0.9
    width: parent.width * 0.9
    anchors.centerIn: parent
    btnCreate.onClicked: {
      chooseAccountPopup.open()
    }
    btnSelect.onClicked: {
      QmlSystem.setCurrentAccount(accountList.currentItem.itemAddress)
      QmlSystem.goToOverview()
      QmlSystem.setScreen(content, "qml/screens/OverviewScreen.qml")
    }
    btnErase.onClicked: {
      // TODO
    }
  }

  AVMEPopupChooseAccount {
    id: chooseAccountPopup
  }
}

