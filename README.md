# EOS Ranger

> Syncronize your database with any EOSIO-compatible blockchain

## brainstorming/notes

vertices:

- block
- transaction
- action
- account

edges:

- block <-> transaction
- transaction <-> action
- action <-> account

possible queries:

- blocks
  - block_num
  - chain_id
  - irreversible
  - data
- transactions
  - block_num
  - chain_id
  - data
- actions

use cases:

- find blocks by producer
- find actions sent to code account
- find users of code account
- find top RAM traders
- find
