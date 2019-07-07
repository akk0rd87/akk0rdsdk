setx AKKORD_SDK_HOME %~dp0
git config core.autocrlf false
git config core.whitespace -cr-at-eol,trailing-space,space-before-tab,tab-in-indent
copy /Y tools\githooks\pre-commit .git\hooks