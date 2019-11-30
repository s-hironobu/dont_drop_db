# dont_drop_db

`dont_drop_db` prohibits to drop the databases listed in the dont_drop_db.list parameter.

## Installation
```
$ cd contrib
$ git clone https://github.com/s-hironobu/dont_drop_db.git
$ cd dont_drop_db
$ make && make install
```

Add the line shown below in your postgresql.conf.

```
shared_preload_libraries = 'dont_drop_db'
```

After starting your server, issue `CREATE EXTENSION` statement shown below.

```
postgres=# CREATE EXTENSION dont_drop_db;
```

## Configuration Parameters

 - dont_drop_db.list : The list of databases that you want to prohibit to drop. The default is 'postgres,template0,template1'. If you set 'all', all databases cannot be dropped. Note that if you set only spaces such as " ", this extension does not prohibit to drop any database.

```
# Examples.
dont_drop_db.list = 'postgres,template0,template1,testdb' # prohibit to drop these databases
dont_drop_db.list = 'all' # prohibit to drop all databases
dont_drop_db.list = ' ' # not prohibit to drop any database
```

## Change Log
 - 1st Dec 2019: Version 1.0 Released.
