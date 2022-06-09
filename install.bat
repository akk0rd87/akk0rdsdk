setx AKKORD_SDK_HOME %~dp0
copy /Y tools\githooks\pre-commit .git\hooks
git config user.name "Amir"
git config user.email "amkad87@gmail.com"