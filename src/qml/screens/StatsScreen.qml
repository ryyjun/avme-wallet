import QtQuick 2.9
import QtQuick.Controls 2.2

import "qrc:/qml/components"

/**
 * Screen for listing an Account's stats and transaction history,
 * as well as several transaction actions for it
 */

Item {
  id: statsScreen

  // Timer for reloading the Account balances
  Timer {
    id: listReloadTimer
    interval: 1000
    repeat: true
    onTriggered: reloadBalances()
  }

  Component.onCompleted: {
    reloadTransactions()
    reloadBalances()
    listReloadTimer.start()
  }

  // Helpers for managing screen data
  function reloadTransactions() {
    historyModel.clear()
    var txList = System.listAccountTransactions(System.getTxSenderAccount())
    if (txList != null) {
      for (var i = (txList.length - 1); i >= 0; i--) {
        historyModel.append(JSON.parse(txList[i]))
      }
    }
  }

  function reloadBalances() {
    var obj = System.getAccountBalances(System.getTxSenderAccount())
    balanceCoinText.text = (obj.balanceAVAX) ? obj.balanceAVAX : "Loading..."
    balanceTokenText.text = (obj.balanceAVME) ? obj.balanceAVME : "Loading..."
    balanceLPFreeText.text = (obj.balanceLPFree) ? obj.balanceLPFree : "Loading..."
    balanceLPLockedText.text = (obj.balanceLPLocked) ? obj.balanceLPLocked : "Loading..."
    if (obj.balanceAVAX && obj.balanceAVME && obj.balanceLPFree && obj.balanceLPLocked) {
      btnSendCoinTx.enabled = true
      btnSendTokenTx.enabled = true
      btnExchange.enabled = true
      btnStaking.enabled = true
    } else {
      btnSendCoinTx.enabled = false
      btnSendTokenTx.enabled = false
      btnExchange.enabled = false
      btnStaking.enabled = false
    }
  }

  // Background icon
  Image {
    id: bgIcon
    width: 256
    height: 256
    anchors.centerIn: parent
    fillMode: Image.PreserveAspectFit
    source: "qrc:/img/avme_logo.png"
  }

  // Account and copy/change buttons
  Row {
    id: statsHeaderRow
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
      topMargin: 10
      leftMargin: 10
    }
    spacing: 10

    AVMEButton {
      id: btnChangeAccount
      width: (parent.width / 6) - parent.spacing
      text: "Change Account"
      onClicked: {
        listReloadTimer.stop()
        System.setFirstLoad(true)
        System.setScreen(content, "qml/screens/AccountsScreen.qml")
      }
    }
    AVMEButton {
      id: btnRefreshHistory
      width: (parent.width / 6) - parent.spacing
      text: "Refresh History"
      onClicked: reloadTransactions()
    }
    Text {
      id: accountText
      width: (parent.width / 3) - parent.spacing
      horizontalAlignment: Text.AlignHCenter
      text: "Stats for the Account:<br><b>" + System.getTxSenderAccount() + "</b>"
    }
    AVMEButton {
      id: btnCopyAddress
      width: (parent.width / 6) - parent.spacing
      Timer { id: textTimer; interval: 2000 }
      enabled: (!textTimer.running)
      text: (!textTimer.running) ? "Copy to Clipboard" : "Copied!"
      onClicked: {
        System.copyToClipboard(System.getTxSenderAccount())
        textTimer.start()
      }
    }
    AVMEButton {
      id: btnViewKey
      width: (parent.width / 6) - parent.spacing
      text: "View Private Key"
      onClicked: {
        viewPrivKeyPopup.account = System.getTxSenderAccount()
        viewPrivKeyPopup.open()
      }
    }
  }

  // List of sent transactions (left)
  Rectangle {
    id: listRect
    width: (parent.width * 0.35)
    anchors {
      top: statsHeaderRow.bottom
      bottom: parent.bottom
      left: parent.left
      margins: 10
    }
    radius: 5
    color: "#4458A0C9"

    AVMETxHistoryList {
      id: historyList
      anchors.fill: parent
      model: ListModel { id: historyModel }
    }
  }

  // Account stats and actions (top right)
  Rectangle {
    id: statsRect
    height: (parent.height * 0.3)
    anchors {
      top: statsHeaderRow.bottom
      left: listRect.right
      right: parent.right
      margins: 10
    }
    radius: 5
    color: "#CC9A4FAD"

    Text {
      id: balanceTitle
      anchors {
        top: parent.top
        horizontalCenter: parent.horizontalCenter
        margins: 10
      }
      text: "Account Balances"
    }

    Text {
      id: balanceCoinText
      anchors {
        top: balanceTitle.bottom
        left: parent.left
        margins: 10
      }
      font.bold: true
      font.pointSize: 14.0
    }

    Text {
      id: balanceCoinType
      anchors {
        top: balanceTitle.bottom
        right: parent.right
        margins: 10
      }
      font.pointSize: 14.0
      text: System.getCurrentCoin()
    }

    Text {
      id: balanceTokenText
      anchors {
        top: balanceCoinText.bottom
        left: parent.left
        margins: 10
      }
      font.bold: true
      font.pointSize: 14.0
    }

    Text {
      id: balanceTokenType
      anchors {
        top: balanceCoinText.bottom
        right: parent.right
        margins: 10
      }
      font.pointSize: 14.0
      text: System.getCurrentToken()
    }

    Text {
      id: balanceLPFreeText
      anchors {
        top: balanceTokenText.bottom
        left: parent.left
        margins: 10
      }
      font.bold: true
      font.pointSize: 14.0
    }

    Text {
      id: balanceLPFreeType
      anchors {
        top: balanceTokenText.bottom
        right: parent.right
        margins: 10
      }
      font.pointSize: 14.0
      text: "LP (Free)"
    }

    Text {
      id: balanceLPLockedText
      anchors {
        top: balanceLPFreeText.bottom
        left: parent.left
        margins: 10
      }
      font.bold: true
      font.pointSize: 14.0
    }

    Text {
      id: balanceLPLockedType
      anchors {
        top: balanceLPFreeText.bottom
        right: parent.right
        margins: 10
      }
      font.pointSize: 14.0
      text: "LP (Locked)"
    }

    AVMEButton {
      id: btnSendCoinTx
      anchors {
        bottom: parent.bottom
        left: parent.left
        margins: 10
      }
      width: (parent.width / 4) - anchors.margins
      text: "Send " + System.getCurrentCoin()
      onClicked: {
        System.setTxTokenFlag(false)
        listReloadTimer.stop()
        System.setTxSenderCoinAmount(balanceCoinText.text)
        System.setScreen(content, "qml/screens/CoinTransactionScreen.qml")
      }
    }
    AVMEButton {
      id: btnSendTokenTx
      anchors {
        bottom: parent.bottom
        left: btnSendCoinTx.right
        margins: 10
      }
      width: (parent.width / 4) - anchors.margins
      text: "Send " + System.getCurrentToken()
      onClicked: {
        System.setTxTokenFlag(true)
        listReloadTimer.stop()
        System.setTxSenderCoinAmount(balanceCoinText.text)
        System.setTxSenderTokenAmount(balanceTokenText.text)
        System.setScreen(content, "qml/screens/TokenTransactionScreen.qml")
      }
    }
    AVMEButton {
      id: btnExchange
      anchors {
        bottom: parent.bottom
        left: btnSendTokenTx.right
        margins: 10
      }
      width: (parent.width / 4) - anchors.margins
      text: "Exchange"
      onClicked: {
        listReloadTimer.stop()
        System.setTxSenderCoinAmount(balanceCoinText.text)
        System.setTxSenderTokenAmount(balanceTokenText.text)
        System.setScreen(content, "qml/screens/ExchangeScreen.qml")
      }
    }
    AVMEButton {
      id: btnStaking
      anchors {
        bottom: parent.bottom
        left: btnExchange.right
        right: parent.right
        margins: 10
      }
      width: (parent.width / 4) - anchors.margins
      text: "Staking"
      onClicked: {
        listReloadTimer.stop()
        System.setScreen(content, "qml/screens/StakingScreen.qml")
      }
    }
  }

  // Transaction details (bottom right)
  Rectangle {
    id: txDetailsRect
    height: (parent.height * 0.6)
    anchors {
      top: statsRect.bottom
      left: listRect.right
      right: parent.right
      bottom: parent.bottom
      margins: 10
    }
    radius: 5
    color: "#CC9A4FAD"

    Text {
      id: detailsTitle
      anchors {
        top: parent.top
        horizontalCenter: parent.horizontalCenter
        margins: 10
      }
      text: (historyList.currentItem) ? "Transaction Details" : "No transactions made yet"
    }

    Text {
      id: detailsText
      anchors {
        top: detailsTitle.bottom
        left: parent.left
        right: parent.right
        margins: 10
      }
      elide: Text.ElideRight
      text: (historyList.currentItem)
      ? "<b>Operation:</b> " + historyList.currentItem.itemOperation + "<br><br>"
      + "<b>From:</b> " + historyList.currentItem.itemFrom + "<br><br>"
      + "<b>To:</b> " + historyList.currentItem.itemTo + "<br><br>"
      + "<b>Value:</b> " + historyList.currentItem.itemValue + "<br><br>"
      + "<b>Gas:</b> " + historyList.currentItem.itemGas + "<br><br>"
      + "<b>Price:</b> " + historyList.currentItem.itemPrice + "<br><br>"
      + "<b>Timestamp:</b> " + historyList.currentItem.itemDateTime + "<br><br>"
      + "<b>Confirmed:</b> " + historyList.currentItem.itemConfirmed
      : ""
    }

    AVMEButton {
      id: btnOpenLink
      anchors {
        bottom: parent.bottom
        left: parent.left
        right: parent.right
        margins: 10
      }
      enabled: (historyList.currentItem)
      text: "Open Transaction in Block Explorer"
      onClicked: Qt.openUrlExternally(historyList.currentItem.itemTxLink)
    }
  }

  // Popup for viewing the Account's private key
  AVMEPopupViewPrivKey {
    id: viewPrivKeyPopup
    showBtn.onClicked: {
      if (System.checkWalletPass(pass)) {
        viewPrivKeyPopup.showPrivKey()
      } else {
        viewPrivKeyPopup.showErrorMsg()
      }
    }
  }
}

