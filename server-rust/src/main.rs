extern {
    // Our C function definitions!
    pub fn srv_main();
}
extern crate freeciv_srv;
extern crate freeciv;

fn main() {
    unsafe {
        srv_main();
    }
    //fc_interface_init_server();

    //srv_prepare();

    /* Run server loop */
    //do {
    //    set_server_state(S_S_INITIAL);

        /* Load a script file. */
//        if (NULL != srvarg.script_filename) {
            /* Adding an error message more here will duplicate them. */
  //          (void) read_init_script(NULL, srvarg.script_filename, TRUE, FALSE);
    //    }

      //  (void) aifill(game.info.aifill);
        //if (!game_was_started()) {
//            event_cache_clear();
//        }

//        log_normal(_("Now accepting new client connections on port %d."),
//                   srvarg.port);
        /* Remain in S_S_INITIAL until all players are ready. */
//        while (S_E_FORCE_END_OF_SNIFF != server_sniff_all_input()) {
            /* When force_end_of_sniff is used in pregame, it means that the server
             * is ready to start (usually set within start_game()). */
//        }

//        if (S_S_RUNNING > server_state()) {
            /* If restarting for lack of players, the state is S_S_OVER,
             * so don't try to start the game. */
//            srv_ready(); /* srv_ready() sets server state to S_S_RUNNING. */
//            srv_running();
//            srv_scores();
//        }

        /* Remain in S_S_OVER until players log out */
//        while (conn_list_size(game.est_connections) > 0) {
  //           server_sniff_all_input();
//        }

//        if (game.info.timeout == -1 || srvarg.exit_on_end) {
            /* For autogames or if the -e option is specified, exit the server. */
//            server_quit();
//        }

        /* Reset server */
//        server_game_free();
//        server_game_init();
//        mapimg_reset();
//        load_rulesets(NULL, TRUE);
//        game.info.is_new_game = TRUE;
//  }  while (TRUE);

    /* Technically, we won't ever get here. We exit via server_quit. */
}

