import QtQuick 2.9
import QtQuick.Controls 2.2

/**
 * Popup for (un)staking LP. Has to be opened manually.
 * Has the following items:
 * - "isStaking": bool for knowing if it's staking or unstaking
 * - "lpAmount": the amount
 * - "gasLimit": self-explanatory
 * - "gasPrice": self-explanatory
 * - "pass": the Wallet password input
 * - "confirmBtn.onClicked": what to do when confirming the action
 * - "setTxData(isStaking, lpAmount, gasLimit, gasPrice)": set tx data for display
 * - "showErrorMsg()": self-explanatory
 * - "clean()": helper function to clean up inputs/data
 */

Popup {
  id: confirmStakePopup
  property bool isStaking
  property string lpAmount
  property string gasLimit
  property string gasPrice
  property alias pass: passInput.text
  property alias confirmBtn: btnConfirm

  function setTxData(isStaking, lpAmount, gasLimit, gasPrice) {
    confirmStakePopup.isStaking = isStaking
    confirmStakePopup.lpAmount = lpAmount
    confirmStakePopup.gasLimit = gasLimit
    confirmStakePopup.gasPrice = gasPrice
  }

  function showErrorMsg() {
    passTextTimer.start()
  }

  function clean() {
    lpAmount = gasLimit = gasPrice = passInput.text = ""
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
      text: "You will " + ((isStaking) ? "stake" : "withdraw") + " <b>" + lpAmount + " LP</b>"
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
          confirmStakePopup.clean()
          confirmStakePopup.close()
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