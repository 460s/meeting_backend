# Стягиваем репозиторий к себе
- Создай каталог src - `mkdir src`
- Перейди в него - `cd src`
- Скачай себе репозиторий - `git clone https://github.com/460s/meeting_backend.git`
- Перейди в склонированный каталог - `cd meeting_backend`
  
# Настройка git
Правильный программист настраивает свой инструмент. Для гита нужно отредактировать файл ~/.gitconfig

Вот шаблон, вместо имени и мыла грута напиши свои (под этим именем ты будешь фигурировать в истории):
```
[user]
       name = i am groot
       email = groot@example.com
[alias]
       co = checkout
       ci = commit
       st = status
       br = branch
       hist = log --pretty=format:\"%h %ad | %s%d [%an]\" --graph --date=short
       lg1 = log --graph --all --format=format:'%C(bold blue)%h%C(reset) - %C(bold green)(%ar)%C(reset) %C(white)%s%C(reset) %C(bold white)— %an%C(reset)%C(bold yellow)%d%C(reset)' --abbrev-commit --date=relative
       lg2 = log --graph --all --format=format:'%C(bold blue)%h%C(reset) - %C(bold cyan)%aD%C(reset) %C(bold green)(%ar)%C(reset)%C(bold yellow)%d%C(reset)%n          %C(white)%s%C(reset) %C(bold white)— %an%C(reset)' --abbrev-commit
       lg = !"git lg2"
[push]
       default = simple
[core]
       pager = less -F -X
```
Поздравляю у тебя есть алиасы (или сокращения) для команд гита и ты можешь как белый человек писать:
- `git co` вместо `git checkout`. Используется для переключения между ветками и удаления незафиксированных изменений
- `git ci` вместо `git commit`. Используется для фиксации всех добавленных в stage изменений
- `git st` вместо `git status`. Используется для получения текущего состояния, в каком ты бранче, какие файлы поторогал, какие хочешь закоммитить
- `git br` вместо `git branch`. Используется для просмотра списка бранчей, а так же создания и удаления
- Плюс добавлены команды для просмотра истории в разных форматах: попробуйте их все `git hist`, `git lg1`, `git lg2`

Так же в гите есть команды:
- `git add` добавить файл в stage
- `git reset` убрать файл из stage
- `git diff` посмотреть что поменялось от коммита или stage
- `git pull` стянуть изменения с сервера
- `git push` залить изменения на сервере
- `git clone` склонировать новый репозиторий к себе на комп
- `git blame` найти виноватого, того кто последний трогал эту строку файле

Вообще git очень подробно выводит, что с ним можно сделать при вводе команд.
Например если поменять файл README.md, а затем сказать `git st` то вывод будет такой
```
On branch master
Your branch is up to date with 'origin/master'.

Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git checkout -- <file>..." to discard changes in working directory)

	modified:   README.md

no changes added to commit (use "git add" and/or "git commit -a")
```
И в нем сказано про `git add`, `git checkout`, `git commit -a`. В общем читай то, что выдает git и будет хорошо 

# Едем дальше
Переходим в бранч [hello-conan](https://github.com/460s/meeting_backend/tree/hello)

Сделайте ```git co hello```
