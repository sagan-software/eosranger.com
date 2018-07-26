type t;

module Options = {
  /* https://www.elastic.co/guide/en/elasticsearch/client/javascript-api/current/configuration.html */
  [@bs.deriving abstract]
  type t = {
    [@bs.optional]
    host: string,
    [@bs.optional]
    hosts: array(string),
    [@bs.optional]
    httpAuth: string,
    [@bs.optional]
    log: string,
    [@bs.optional]
    requestTimeout: int,
    [@bs.optional]
    deadTimeout: int,
    [@bs.optional]
    pingTimeout: int,
    [@bs.optional]
    maxSockets: int,
    [@bs.optional]
    keepAlive: bool,
    [@bs.optional]
    keepAliveInterval: int,
    [@bs.optional]
    keepAliveMaxFreeSockets: int,
    [@bs.optional]
    keepAliveFreeSocketTimeout: int,
    [@bs.optional]
    suggestCompression: bool,
    [@bs.optional]
    connectionClass: string,
  };
};

[@bs.module "elasticsearch"] [@bs.new]
external make_ : Options.t => t = "Client";

let make =
    (
      ~host=?,
      ~hosts=?,
      ~httpAuth=?,
      ~log=?,
      ~requestTimeout=?,
      ~deadTimeout=?,
      ~pingTimeout=?,
      ~maxSockets=?,
      ~keepAlive=?,
      ~keepAliveInterval=?,
      ~keepAliveMaxFreeSockets=?,
      ~keepAliveFreeSocketTimeout=?,
      ~suggestCompression=?,
      ~connectionClass=?,
      (),
    ) =>
  Options.t(
    ~host?,
    ~hosts?,
    ~httpAuth?,
    ~log?,
    ~requestTimeout?,
    ~deadTimeout?,
    ~pingTimeout?,
    ~maxSockets?,
    ~keepAlive?,
    ~keepAliveInterval?,
    ~keepAliveMaxFreeSockets?,
    ~keepAliveFreeSocketTimeout?,
    ~suggestCompression?,
    ~connectionClass?,
    (),
  )
  |. make_;

[@bs.send] external ping : (t, 'params) => Js.Promise.t(int) = "";

[@bs.send] external search : (t, 'params) => Js.Promise.t(Js.Json.t) = "";

[@bs.send] external get : (t, 'params) => Js.Promise.t(Js.Json.t) = "";

[@bs.send] external create : (t, 'params) => Js.Promise.t(Js.Json.t) = "";

module Indices = {
  type t;

  [@bs.send]
  external putMapping : (t, 'mapping) => Js.Promise.t(Js.Json.t) = "";
};

[@bs.get] external indices : t => Indices.t = "";
