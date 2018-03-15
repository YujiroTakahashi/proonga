Groongaクライアント
======================
PHP7 bindings for Groonga.

利用方法
------

### Groongaライブラリのインストール ###
```    
$ sudo aptitude install -y libgroonga0 libgroonga-dev
```

### proongaのインストール ###
```    
$ git clone https://github.com/YujiroTakahashi/proonga.git
$ cd ./proonga
$ phpize
$ ./configure
$ make
$ sudo -s
# make install
# cd /etc/php/7.0/mods-available
# echo extension=proonga.so > proonga.ini
# cd /etc/php/7.0/fpm/conf.d
# ln -s /etc/php/7.0/mods-available/proonga.ini ./30-proonga.ini
```

内部でphp_json_decode_exを使用しています。  

PHPのjsonライブラリを読み込んでからproongaを読み込むように  
設定してください。
