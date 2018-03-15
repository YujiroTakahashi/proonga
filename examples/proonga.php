<?php
/*
php -d extension=modules/proonga.so -f examples/proonga.php
*/

/* DB作成・オープン */
$gdb = new Proonga('./db/test.db');

/**/
echo "table_create --name Sample --flags TABLE_HASH_KEY --key_type ShortText\n";
$res = $gdb->exec("table_create", [
    'name'     => 'Users',
    'flags'    => 'TABLE_HASH_KEY',
    'key_type' => 'ShortText'
]);
var_dump($res);
echo "\n";

echo "column_create --table Users --name name --flags COLUMN_SCALAR --type ShortText\n";
$res = $gdb->exec("column_create", [
    'table' => 'Users',
    'name'  => 'name',
    'flags' => 'COLUMN_SCALAR',
    'type'  => 'ShortText'
]);
var_dump($res);
echo "\n";

echo 'load --table Users  --input_type json
[
    {"_key":"alice", "name":"Alice"},
    {"_key":"march", "name":"March Hare"}
]'."\n";

$valus = [
    ['_key' => 'alice', 'name' => 'Alice'],
    ['_key' => 'march', 'name' => 'March Hare'],
];
$res = $gdb->exec("load", [
    'table'      => 'Users',
    'input_type' => 'json',
    'values'     => json_encode($valus)
]);
var_dump($res);
echo "\n";

echo "select --table Users  --limit 10\n";
$res = $gdb->exec("select", [
    'table'=> 'Users'
]);
print_r($res);
echo "\n";

echo "=============== status ===============\n";
$res = $gdb->exec("status");
print_r($res);
echo "\n";

/**/
