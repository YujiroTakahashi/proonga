Groongaクライアント
======================
PHP7 / PHP8 bindings for Groonga.

---

## 目次

* [利用方法](#install)
* [Docker環境実行](#docker)
* [Proongaクラス](#class00)


## <a name="install">利用方法

### Groongaライブラリのインストール ###
```    
$ sudo aptitude install -y libgroonga0 libgroonga-dev
```

### proongaのインストール ###
```    
$ git clone https://github.com/YujiroTakahashi/proonga.git
$ cd ./proonga/proonga
$ phpize
$ ./configure
$ make -j
$ sudo -s
# make install
# cd /etc/php/8.0/mods-available
# echo extension=proonga.so > proonga.ini
# cd /etc/php/8.0/fpm/conf.d
# ln -s /etc/php/8.0/mods-available/proonga.ini ./30-proonga.ini
```

内部でphp_json_decode_exを使用しています。  

PHPのjsonライブラリを読み込んでからproongaを読み込むように  
設定してください。  


## <a name="docker">Docker環境実行
```
docker-compose build
docker-compose run --user croco shell php8 /opt/examples/proonga.php
```

## <a name="class00">クラス  

* [Proonga::__construct](#class10)― コンストラクタGroongaの初期化など
* [Proonga::exec](#class20) ― 各コマンドの実行


## <a name="class10"> コンストラクタGroongaの初期化など

```php
public Proonga::__construct(string $path)
```
  
**引数**
  
* $path ・・・・・・ GroongaDBの保存先


## <a name="class20"> 各コマンドの実行

```php
public Proonga::exec(string $command, array $params)
```
  
**引数**
  
* $command ・・・・・・ Groongaコマンド
* $params ・・・・・・ 各コマンドの引数


### <a name="class11">接続 ###

```php
<?php
/* オブジェクト生成 */
$gdb = new Proonga('./db/test.db');
```

### <a name="class21">テーブルの作成 ###

```php
/* table_create --name Users --flags TABLE_HASH_KEY --key_type ShortText */
$res = $gdb->exec("table_create", [
    'name'     => 'Users',
    'flags'    => 'TABLE_HASH_KEY',
    'key_type' => 'ShortText'
]);
```

### <a name="class22">カラムの作成 ###

```php
/* column_create --table Users --name name --flags COLUMN_SCALAR --type ShortText */
$res = $gdb->exec("column_create", [
    'table' => 'Users',
    'name'  => 'name',
    'flags' => 'COLUMN_SCALAR',
    'type'  => 'ShortText'
]);
```

### <a name="class23">データのロード ###

```php
/*
load --table Users --input_type json
[
    {"_key":"alice", "name":"Alice"},
    {"_key":"march", "name":"March Hare"}
]
*/
$valus = [
    ['_key' => 'alice', 'name' => 'Alice'],
    ['_key' => 'march', 'name' => 'March Hare'],
];
$res = $gdb->exec("load", [
    'table'      => 'Users',
    'input_type' => 'json',
    'values'     => json_encode($valus)
]);

```

### <a name="class24">データ一覧の取得 ###

```php
/* select --table Users  --limit 10" */
$res = $gdb->exec("select", [
    'table'=> 'Users'
]);
print_r($res);

```

### 出力結果 ###

```
Array
(
    [0] => Array
        (
            [0] => Array
                (
                    [0] => 2
                )

            [1] => Array
                (
                    [0] => Array
                        (
                            [0] => _id
                            [1] => UInt32
                        )

                    [1] => Array
                        (
                            [0] => _key
                            [1] => ShortText
                        )

                    [2] => Array
                        (
                            [0] => name
                            [1] => ShortText
                        )

                )

            [2] => Array
                (
                    [0] => 1
                    [1] => alice
                    [2] => Alice
                )

            [3] => Array
                (
                    [0] => 2
                    [1] => march
                    [2] => March Hare
                )

        )

)
```

    
    

ライセンス
----------
Copyright &copy; 2014 Yujiro Takahashi  
Licensed under the [MIT License][MIT].  
Distributed under the [MIT License][MIT].  

[MIT]: http://www.opensource.org/licenses/mit-license.php
