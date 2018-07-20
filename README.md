```
curl -X POST --data-binary @- --dump - http://root:openSesame@localhost:8529/_db/testing/_api/document/testcollection?overwrite=true <<EOF
{ "_key": "test", "Hello": "World" }
EOF
```

```
curl -X POST --data-binary @- --dump -
https://api-prd.eosflare.io/chain/get_actions <<EOF
{ "account": "g4ydegenesis" }
EOF
```

1.  [x] setup database
    1.  [x] get existing db state. possible states:
        1.  [x] db does not exist
        2.  [x] db exists, collection does not exist
        3.  [x] db and collection exist
2.  [x] begin sync loop
    1.  [x] get the highest block number in the db
        1.  [x] if no block number is found, assume it is 0 (fetch starts at block 1)
    2.  [x] fetch chain info from all http endpoints
    3.  [ ] filter out endpoints that return a bad status or have an unwanted chain id
    4.  [x] select highest head_block_num
    5.  [ ] query the DB for reversible blocks that need to be updated:
        1.  [ ] irreverisble == false (still marked as reversible)
        2.  [ ] block_num < last_irreversible_block_num (no longer reversible)
    6.  [ ] add block numbers to the queue: head_block_num - latest_db_block_num
    7.  [ ] detect missing blocks in the DB
        1.  [ ] if the largest block num in the DB is higher than the total document count in the blocks collection, then we are missing blocks
        2.  [ ] if we are missing blocks then fetch all block numbers and try to find the unaccounted for block numbers.
        3.  [ ] exit loop as soon as missing block numbers are found
    8.  [ ] begin fetch loop
        1.  [ ] if there are no block numbers in the event queue, then skip to the next step
        2.  [ ] distribute all block numbers in the queue evenly amongst all http endpoints, weighted by number of errors previously returned
        3.  [ ] fetch blocks from the http endpoints, throttled to ~750ms
            1.  [ ] if the status is good and the content type is right:
                1.  [ ] block.\_key = block.block_num
                2.  [ ] block.irreversible = block.block_num <= info.last_irreversible_block_num
                3.  [ ] save the block to arango, log any errors
            2.  [ ] if a bad status or other errors are returned, add the block number back to the queue and increment the error count for the http endpoint
    9.  [ ] restart sync loop

possible issues:

- checking for missing block numbers in the DB
- keeping the DB updated with real-time info
