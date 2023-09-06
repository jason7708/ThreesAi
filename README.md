# ThreesAi
Change from Theory of computer game hw 2018  

[Game Link](http://threesjs.com/)  
AI programs that play Threes, the origin of other 2048-like games.
* Init weight
```
./three.exe --total=1000000 --block=1000 --limit=100 --play="init save=new_weights.bin" --evil="seed=1234"
```
* Load weight
```
./three.exe --total=1000000 --block=1000 --limit=100 --play="load=weights.bin save=new_weights.bin" --evil="seed=1234"
```
1000 games test
![image](https://github.com/jason7708/ThreesAi/assets/44204022/539b60fe-53ef-4807-ac97-64e90ceb3219)
