let dbUser = "root";
let dbPass = "openSesame";
let dbName = "eos";
let throttleTime = 1500;
let responseTimeMultiplier = 1.5;
let maxBlocksPerCycle = 5000;
let reportStatsTimeout = 30000;
let chainId = "aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906";
let endpoints = [|
  "http://104.156.59.166:8888",
  "http://149.202.180.100",
  /* "http://api-1.eosgreen.io:8888", */
  /* "http://api-eos.blckchnd.com", */
  /* "http://api-mainnet.starteos.io", */
  /* "http://api-mainnet1.starteos.io", */
  "http://api.bitmars.one",
  "http://api.bp.antpool.com",
  /* "http://api.bp.fish", */
  /* "http://api.cypherglass.com", */
  /* "http://api.dutcheos.io", */
  /* "http://api.eos.store", */
  "http://api.eosargentina.io",
  /* "http://api.eosbeijing.one", */
  /* "http://api.eoscleaner.com", */
  /* "http://api.eoseco.com", */
  "http://api.eosgeneva.io",
  "http://api.eosmedi.com",
  "http://api.eosn.io",
  "http://api.eosnewyork.io",
  /* "http://api.eosrio.io", */
  "http://api.eossocal.io",
  "http://api.eossweden.eu",
  "http://api.eossweden.se",
  "http://api.eostitan.com",
  "http://api.eosuk.io",
  /* "http://api.helloeos.com.cn", */
  /* "http://api.hkeos.com", */
  /* "http://api.jeda.one:8888", */
  /* "http://api.main-net.eosnodeone.io", */
  "http://api.main.alohaeos.com",
  /* "http://api.mainnet.eospace.io", */
  "http://api.proxy1a.sheos.org",
  "http://api.tokenika.io",
  "http://api1.eosasia.one",
  /* "http://api1.eosdublin.io", */
  "http://api1.eostheworld.io:4865",
  /* "http://api2.eos.ren:8883", */
  "http://api2.eosgeneva.io",
  /* "http://api2.eosnairobi.io:8898", */
  /* "http://api2.franceos.fr", */
  "http://blockgenesys.com:8888",
  "http://bp.blockchainlab.me:9090",
  /* "http://bp.cryptolions.io", */
  /* "http://br.eosrio.io", */
  /* "http://cpt.eosio.africa", */
  /* "http://dns1-rpc.oraclechain.io:58888",
     "http://dns2-rpc.oraclechain.io:58888", */
  "http://eos-api.privex.io",
  "http://eos-bp.bitfinex.com:8888",
  /* "http://eos.genesis-mining.com:8443", */
  "http://eos.greymass.com",
  "http://eosbp-0.atticlab.net",
  /* "http://eosfull.kuna.io", */
  "http://eu.eosdac.io",
  "http://fn001.eossv.org:80",
  /* "http://jhb.eosio.africa", */
  /* "http://mainnet.bpnode.com:8888", */
  "http://mainnet.eosarabia.org:2052",
  "http://mainnet.eoscalgary.io",
  "http://mainnet.eoscanada.com",
  /* "http://mainnet.eoscannon.io", */
  /* "http://mainnet.genereos.io", */
  "http://mainnet.libertyblock.io:8888",
  "http://node.eosflare.io",
  /* "http://node.eosmeso.io", */
  "http://node.eosvenezuela.io:8888",
  "http://node1.bp2.io",
  "http://node1.eosamericas.com:8888",
  /* "http://node1.eosphere.io", */
  "http://node1.eosvibes.io",
  /* "http://node1.zbeos.com:8888", */
  /* "http://node2.eosphere.io", */
  /* "http://node2.liquideos.com", */
  /* "http://node2.zbeos.com:8888", */
  /* "http://nodes.eos42.io", */
  /* "http://peer1.eoshuobipool.com:8181",
     "http://peer2.eoshuobipool.com:8181", */
  "http://peer2.eosthu.com:8082",
  /* "http://publicapi-mainnet.eosauthority.com", */
|];
