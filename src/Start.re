let logo = {j|

   ███████╗ ██████╗ ███████╗    ███████╗██╗   ██╗███╗   ██╗ ██████╗
   ██╔════╝██╔═══██╗██╔════╝    ██╔════╝╚██╗ ██╔╝████╗  ██║██╔════╝
   █████╗  ██║   ██║███████╗    ███████╗ ╚████╔╝ ██╔██╗ ██║██║
   ██╔══╝  ██║   ██║╚════██║    ╚════██║  ╚██╔╝  ██║╚██╗██║██║
   ███████╗╚██████╔╝███████║    ███████║   ██║   ██║ ╚████║╚██████╗
   ╚══════╝ ╚═════╝ ╚══════╝    ╚══════╝   ╚═╝   ╚═╝  ╚═══╝ ╚═════╝

|j};

let start = () =>
  Database.setup()
  |> Js.Promise.then_(_ => SyncLoop.initialState |. SyncLoop.start)
  |> Js.Promise.then_(_ => Js.Promise.resolve())
  |> Js.Promise.catch(error =>
       Log.error(__MODULE__, "error", error) |. Js.Promise.resolve
     );

Log.info(__MODULE__, logo, "");
start();
