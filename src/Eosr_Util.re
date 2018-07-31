let formatNumberString =
  Js.String.replaceByRe([%bs.re "/\\B(?=(\\d{3})+(?!\\d))/g"], ",");

let formatInt = int => int |. Js.Int.toString |> formatNumberString;

let decodeResult = (decoder, json) =>
  switch (decoder(json)) {
  | decoded => Belt.Result.Ok(decoded)
  | exception (Json.Decode.DecodeError(msg)) => Belt.Result.Error(msg)
  };

let decodeOption = (decoder, json) =>
  switch (decodeResult(decoder, json)) {
  | Belt.Result.Ok(decoded) => Some(decoded)
  | _ => None
  };

let decodeText = (decoder, text) =>
  text
  |. Json.parse
  |> Js.Option.andThen((. json) => decodeOption(decoder, json));

let parseJsonAsPromise = text =>
  switch (text |. Js.Json.parseExn) {
  | parsed => Js.Promise.resolve(parsed)
  | exception error => Js.Promise.reject(error)
  };

let decodeJsonAsPromise = (decode, json) =>
  switch (json |. decode) {
  | decoded => Js.Promise.resolve(decoded)
  | exception error => Js.Promise.reject(error)
  };

let parseAndDecodeAsPromise = (decode, text) =>
  text |. parseJsonAsPromise |> Js.Promise.then_(decodeJsonAsPromise(decode));

let promiseToOption = promise =>
  promise
  |> Js.Promise.then_(result => result |. Some |. Js.Promise.resolve)
  |> Js.Promise.catch(_error => None |. Js.Promise.resolve);

let promiseToResult = promise =>
  promise
  |> Js.Promise.then_(result =>
       result |. Belt.Result.Ok |. Js.Promise.resolve
     )
  |> Js.Promise.catch(error =>
       error |. Belt.Result.Error |. Js.Promise.resolve
     );

let onlySome = array =>
  Belt.Array.reduce(
    array,
    [||],
    (results, item) => {
      switch (item) {
      | Some(item) => Js.Array.push(item, results) |> ignore
      | _ => ()
      };
      results;
    },
  );

let onlyOk = array =>
  Belt.Array.reduce(
    array,
    [||],
    (results, item) => {
      switch (item) {
      | Belt.Result.Ok(item) => Js.Array.push(item, results) |> ignore
      | _ => ()
      };
      results;
    },
  );

let chunkArray = (originalArr, chunkSize) => {
  let results = [||];
  let arr = Belt.Array.copy(originalArr);
  while (arr |. Belt.Array.length > 0) {
    let chunk =
      arr |> Js.Array.spliceInPlace(~pos=0, ~remove=chunkSize, ~add=[||]);
    results |> Js.Array.push(chunk) |> ignore;
  };
  results;
};

let chunkPromisesArray = (arr: array('a), fn: 'a => 'b, chunkSize) =>
  Js.Array.reduce(
    (promise, chunk) =>
      promise
      |> Js.Promise.then_(allResults =>
           chunk
           |> Js.Array.map(fn)
           |> Js.Promise.all
           |> Js.Promise.then_(results =>
                Js.Promise.resolve(Js.Array.concat(allResults, results))
              )
         ),
    Js.Promise.resolve([||]),
    chunkArray(arr, chunkSize),
  );

let doNothing = thing => thing;

let timeoutPromise = ms =>
  Js.Promise.make((~resolve, ~reject as _) =>
    Js.Global.setTimeout(() => resolve(. 1), ms) |> ignore
  );