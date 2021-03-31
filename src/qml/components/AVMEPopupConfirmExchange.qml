import QtQuick 2.9
import QtQuick.Controls 2.2

/**
 * Popup for confirming a coin/token exchange. Has to be opened manually.
 * Has the following items:
 * - "fromAmount/toAmount": the coin/token amounts that will be swapped
 * - "fromLabel/toLabel": the coin/token labels
 * - "gasLimit": self-explanatory
 * - "gasPrice": self-explanatory
 * - "pass": the Wallet password input
 * - "confirmBtn.onClicked": what to do when confirming the action
 * - "setTxData(fromAmount, toAmount, fromLabel, toLabel, gasLimit, gasPrice)":
 *   set tx data for display
 * - "showErrorMsg()": self-explanatory
 * - "clean()": helper function to clean up inputs/data
 */

Popup {
  id: confirmExchangePopup
  property string fromAmount
  property string toAmount
  property string fromLabel
  property string toLabel
  property string gasLimit
  property string gasPrice
  property alias pass: passInput.text
  property alias confirmBtn: btnConfirm

  function setTxData(fromAmount, toAmount, fromLabel, toLabel, gasLimit, gasPrice) {
    confirmExchangePopup.fromAmount = fromAmount
    confirmExchangePopup.toAmount = toAmount
    confirmExchangePopup.fromLabel = fromLabel
    confirmExchangePopup.toLabel = toLabel
    confirmExchangePopup.gasLimit = gasLimit
    confirmExchangePopup.gasPrice = gasPrice
  }

  function showErrorMsg() {
    passTextTimer.start()
  }

  function clean() {
    fromAmount = toAmount = fromLabel = toLabel = gasLimit = gasPrice = passInput.text = ""
  }

  width: window.width / 2
  height: window.height / 2
  x: (window.width / 2) - (width / 2)
  y: (window.height / 2) - (height / 2)
  modal: true
  focus: true
  padding: 0  // Remove white borders
  closePolicy: Popup.NoAutoClose

  Rectangle {
    anchors.fill: parent
    color: "#9A4FAD"

    // Transaction summary
    Text {
      id: infoText
      anchors {
        horizontalCenter: parent.horizontalCenter
        top: parent.top
        topMargin: parent.height / 8
      }
      horizontalAlignment: Text.AlignHCenter
      text: "You will exchange <b>" + fromAmount + " " + fromLabel + "</b>"
      + "<br>Estimated return: <b>" + toAmount + " " + toLabel + "</b>"
      + "<br>Gas Limit: <b>" + gasLimit + " Wei</b>"
      + "<br>Gas Price: <b>" + gasPrice + " Gwei</b>"
    }

    // Passphrase status text ("enter your pass", or "wrong pass")
    Text {
      id: passText
      anchors {
        horizontalCenter: parent.horizontalCenter
        top: infoText.bottom
        topMargin: 15
      }
      horizontalAlignment: Text.AlignHCenter
      Timer { id: passTextTimer; interval: 2000 }
      text: (!passTextTimer.running)
      ? "Please authenticate to confirm the transaction."
      : "Wrong passphrase, please try again"
    }

    // Passphrase input
    AVMEInput {
      id: passInput
      width: parent.width / 2
      echoMode: TextInput.Password
      passwordCharacter: "*"
      label: "Passphrase"
      placeholder: "Your Wallet's passphrase"
      anchors {
        horizontalCenter: parent.horizontalCenter
        top: passText.bottom
        topMargin: 25
      }
    }

    // Buttons
    Row {
      id: btnRow
      anchors {
        horizontalCenter: parent.horizontalCenter
        bottom: parent.bottom
        bottomMargin: parent.height / 8
      }
      spacing: 10

      AVMEButton {
        id: btnCancel
        text: "Cancel"
        onClicked: {
          confirmExchangePopup.clean()
          confirmExchangePopup.close()
        }
      }
      AVMEButton {
        id: btnConfirm
        text: "Confirm"
        enabled: (passInput.text != "")
      }
    }
  }
}
