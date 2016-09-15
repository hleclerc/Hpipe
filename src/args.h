DESCRIPTION( "High performance Parser Generator" );

BARG( 'l', disp_lexem_graph  , "display lexem graph"                              , false    );
BARG( 'c', disp_char_graph   , "display char graph"                               , false    );
SARG( 'i', disp_inst_graph   , "display instruction graph at specified stages ()" , ""       );
BARG( 'p', disp_inst_pred    , "display instruction predecessors"                 , false    );
BARG( 'f', disp_trans_freq   , "display transition frequency"                     , false    );
BARG( 't', test              , "test sipe files"                                  , false    );
BARG( 'b', benchmark         , "benchmark with training data"                     , false    );
BARG(  0 , boyer_moore       , "Boyer-Moore like optimization"                    , false    );
SARG( 'o', output            , "Name of the output file"                          , 0        );
SARG( 's', style             , "Output style (BUFFER, BEG_END or C_STR)"          , "BUFFER" );
SARG( 'a', args              , "Additionnal parse arguments"                      , 0        );
EARG( beg_files              , "File to compile"                                             );
