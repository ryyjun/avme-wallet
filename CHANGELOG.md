# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.1] - 2021-05-11
### Added
- "Copy to Clipboard" button and "Address:" label in the Account header.
- Confirmation popup before making a transaction.
- Warning message in the confirmation popup when sender and receiver Accounts are the same.
- Project version now shows up in the window bar and the About screen.
- Support for replay protection ([EIP-155](https://eips.ethereum.org/EIPS/eip-155)).

### Changed
- Wallet creation screen should be more intuitive/less confusing now.
  - "View seed" popup doesn't have leftover disabled controls anymore.
  - "View passphrase" checkbox was replaced with a button.
  - "Use default path" checkbox was removed.
  - "Confirm passphrase" input now has a visual check.
  - Folder and passphrase buttons now have icons.
- About popup is now a screen of its own, fitting better with the wallet's design.
- Better color contrast between button states.
- Pangolin's [Graph](https://api.thegraph.com/subgraphs/name/dasconnor/pangolin-dex) endpoint has been changed.
  - The old one is functional but has been deprecated and won't receive further updates.

### Removed
- Boost::log as a dependency (the removal helps with MacOS compiling)

### Fixed
- Market graph legends now shouldn't be cut off anymore (e.g. "05/..." instead of "05/03").
- **Linux:** redefine fontconfig path that made the program hang on startup.
- **Windows:** proper high DPI scaling using QT\_SCALE\_FACTOR.
- Fiat pricings in the overview are now properly rounded to two decimals.
  - This fixes balances being shown as scientific notations (e.g. "$3.4717e-16" instead of "$3.47").
- Gas checkboxes now don't lose their predefined values anymore when clicking too fast.

## [0.1.0] - 2021-05-01
### Added
- Initial open beta release.
