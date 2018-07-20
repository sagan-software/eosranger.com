// Generated by BUCKLESCRIPT VERSION 4.0.0, PLEASE EDIT WITH CARE
'use strict';

var Url = require("url");
var Json = require("@glennsl/bs-json/src/Json.bs.js");
var Util = require("./Util.js");
var Npmlog = require("npmlog");
var Request = require("./Request.js");
var Eos_Chain = require("@sagan-software/bs-eos/src/Eos_Chain.js");
var Belt_Array = require("bs-platform/lib/js/belt_Array.js");
var Json_encode = require("@glennsl/bs-json/src/Json_encode.bs.js");
var Js_primitive = require("bs-platform/lib/js/js_primitive.js");

function initialState(endpoint) {
  return /* record */[
          /* endpoint */endpoint,
          /* info */undefined,
          /* averageResponseTime */1,
          /* numErrors */0,
          /* numSuccess */0
        ];
}

function initialStates(endpoints) {
  return Belt_Array.map(endpoints, initialState);
}

function updateInfoForState(state) {
  return Util.promiseToResult(Request.make(new Url.URL("/v1/chain/get_info", state[/* endpoint */0]).toString(), undefined, undefined, undefined, 5000, undefined, undefined, /* () */0).then((function (response) {
                      return Util.parseAndDecodeAsPromise(Eos_Chain.Info[/* decode */0], response.body);
                    }))).then((function (result) {
                if (result.tag) {
                  Npmlog.error("Endpoints", "BAD get_info:", state[/* endpoint */0]);
                  return Promise.resolve(/* record */[
                              /* endpoint */state[/* endpoint */0],
                              /* info */undefined,
                              /* averageResponseTime */state[/* averageResponseTime */2],
                              /* numErrors */state[/* numErrors */3] + 1 | 0,
                              /* numSuccess */state[/* numSuccess */4]
                            ]);
                } else {
                  return Promise.resolve(/* record */[
                              /* endpoint */state[/* endpoint */0],
                              /* info */result[0],
                              /* averageResponseTime */state[/* averageResponseTime */2],
                              /* numErrors */state[/* numErrors */3],
                              /* numSuccess */state[/* numSuccess */4] + 1 | 0
                            ]);
                }
              }));
}

function updateInfoForStates(states) {
  return Promise.all(Belt_Array.map(states, updateInfoForState));
}

function getLargestBlockNums(states) {
  return Belt_Array.reduce(Util.onlySome(Belt_Array.map(states, (function (state) {
                        return state[/* info */1];
                      }))), /* record */[
              /* head */0,
              /* irreversible */0
            ], (function (largestBlockNums, info) {
                var match = info[/* headBlockNum */1] > largestBlockNums[/* head */0];
                if (match) {
                  return /* record */[
                          /* head */info[/* headBlockNum */1],
                          /* irreversible */info[/* lastIrreversibleBlockNum */2]
                        ];
                } else {
                  return largestBlockNums;
                }
              }));
}

function onlyWithInfo(states) {
  return Belt_Array.reduce(states, /* array */[], (function (result, state) {
                var match = state[/* info */1];
                if (match !== undefined) {
                  result.push(/* tuple */[
                        state,
                        match
                      ]);
                }
                return result;
              }));
}

function getBlock(state, num) {
  return Util.promiseToResult(Request.make(new Url.URL("/v1/chain/get_block", state[/* endpoint */0]).toString(), "POST", undefined, Json.stringify(Json_encode.object_(/* :: */[
                              /* tuple */[
                                "block_num_or_id",
                                num
                              ],
                              /* [] */0
                            ])), 5000, undefined, undefined, /* () */0).then((function (response) {
                      return Util.parseJsonAsPromise(response.body);
                    }))).then((function (result) {
                if (result.tag) {
                  Npmlog.error("Endpoints", "BAD get_block:", state[/* endpoint */0]);
                  return Promise.resolve(/* record */[
                              /* state : record */[
                                /* endpoint */state[/* endpoint */0],
                                /* info */state[/* info */1],
                                /* averageResponseTime */state[/* averageResponseTime */2],
                                /* numErrors */state[/* numErrors */3] + 1 | 0,
                                /* numSuccess */state[/* numSuccess */4]
                              ],
                              /* num */num,
                              /* block */undefined
                            ]);
                } else {
                  return Promise.resolve(/* record */[
                              /* state : record */[
                                /* endpoint */state[/* endpoint */0],
                                /* info */state[/* info */1],
                                /* averageResponseTime */state[/* averageResponseTime */2],
                                /* numErrors */state[/* numErrors */3],
                                /* numSuccess */state[/* numSuccess */4] + 1 | 0
                              ],
                              /* num */num,
                              /* block */Js_primitive.some(result[0])
                            ]);
                }
              }));
}

exports.initialState = initialState;
exports.initialStates = initialStates;
exports.updateInfoForState = updateInfoForState;
exports.updateInfoForStates = updateInfoForStates;
exports.getLargestBlockNums = getLargestBlockNums;
exports.onlyWithInfo = onlyWithInfo;
exports.getBlock = getBlock;
/* url Not a pure module */