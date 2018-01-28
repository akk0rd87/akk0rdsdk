call rmdir .git /s
call git init
call git add .
call git commit -m "Initial commit"

call git remote add origin https://akk0rd87@bitbucket.org/akk0rd87/akk0rdsdk.git
call git push -u --force origin master