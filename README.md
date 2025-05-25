# Pintos lab3
## Download

1. Download the zip file from `<>Code`
> [!NOTE] 據說直接git clone會有問題，才改成下載zip file.
> 也可以試試看直接git clone, 那麼Setup git的部分就不用`git init`.

2. unzip the zip file

## Setup git
1. In your terminal, navigate to the directory (.../pintos\_lab3)
2. Run the following command
```bash
git init
git remote add origin https://github.com/matxtam/pintos_lab3.git
```

## Start Docker (First time)
1. Copy `docker-compose-example.yaml` to `docker-compose.yaml` and modify the path inside (IMPORTANT!)
2. start docker engine
3. run at the root of this folder
```bash
docker-compose up -d
```

## Start Docker (After first time)
1. Start docker engine
2. run at anywhere
```bash
docker start pintos
```

## Compile and makecheck
```bash
docker exec -it pintos bash
cd pintos/vm
make
cd build
make check
```

## Push to github 
at the root of the folder:
```
git add .
git commit -m "your commit message"
git pull
# (you might need to solve conflicts here)
# (After solving conflict, you have to git commit again)
git push origin master
```
