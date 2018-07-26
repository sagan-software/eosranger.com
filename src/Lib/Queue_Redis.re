module type Info = {let client: Redis.t; let name: string;};

module Make = (I: Info) : Types.Q => {
  let count = () => I.client |. Redis.scard(I.name);
  let push = nums =>
    I.client |. Redis.sadd(I.name, nums |. Belt.Array.map(string_of_int));
  let pop = () =>
    I.client
    |. Redis.spop(I.name)
    |> Js.Promise.then_(num =>
         num
         |. Js.Nullable.toOption
         |. Belt.Option.map(int_of_string)
         |. Js.Promise.resolve
       );
};
