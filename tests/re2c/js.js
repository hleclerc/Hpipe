const js_tokens_matcher = require( "js-tokens" );

const txt = `
let variants = [], img_names = [];
for( const arg of process.argv.slice( 2 ) ) {
    if ( arg.startsWith( "--" ) ) {
        const ine = arg.indexOf( "=" );
        if ( ine >= 0 ) // && vn.indexOf( arg.substring( 2, ine ) ) >= 0
            variants.push( { name: arg.substring( 2, ine - 1 ), values: arg.substring( ine + 1 ).split( "," ) } );
    } else
        img_names.push( arg );
}

function prefix( ext ) {
    switch ( ext.toLowerCase() ) {
        case ".jpeg":
            return -data:image/jpg;base64,-;
        default:
            return -data:image/*{ ext.slice( 1 ) };base64,-;
    }
}

function explore_variants( lst, var_name, img_name ) {
    if ( lst.length == variants.length ) {
        if ( lst.length == 0 )
            return console.log( -module.exports*{ var_name.length ? "." + var_name : "" } = "*{ prefix( path.extname( img_name ) ) }*{ fs.readFileSync( img_name ).toString( 'base64' ) }";- );
        // check image magick is present
        const rp = cp.spawnSync( "nsmake", [ "prerequ", "convert" ], { stdio: [ 0, 2, 2 ] } );
        if ( rp.error || rp.status )
            process.exit( 1 );
        // launch convert
        return tmp.file( {}, ( err, tmp_name, fd, cleanupCallback ) => {
            let convert_args = [ img_name ];
            for( const v of lst ) {
                if ( v.value.length )
                    convert_args.push( "-" + v.name, v.value );
            }
            convert_args.push( tmp_name );

            const rp = cp.spawn( "convert", convert_args, { stdio: [ 0, 2, 2 ] } );
            rp.on( 'error', err => {
                cleanupCallback();
                process.exit( 1 );
            } );
            rp.on( 'close', code => {
                if ( code ) {
                    cleanupCallback();
                    process.exit( 1 );
                }
                console.log( -module.exports.*{ var_name } = "*{ prefix( path.extname( img_name ) ) }*{ fs.readFileSync( tmp_name ).toString( 'base64' ) }";- );
                cleanupCallback();
            } );
        } );
    }
    for( const value of variants[ lst.length ].values ) {
        explore_variants( [ ...lst, { name: variants[ lst.length ].name, value } ], var_name + (
            value.length ? ( var_name.length ? "__" : "" ) +-*{ variants[ lst.length ].name }_*{ value }- : ''
        ), img_name );
    }
}

for( const img_name of img_names )
    explore_variants( [], img_names.length > 1 ? encodeURIComponent( path.basename( img_name, path.extname( img_name ) ) ) : "", img_name );
`

let cpt = 0;
for( let token of txt.match( js_tokens_matcher ) ) {
    console.log( token );
    ++cpt;
}
console.log( cpt );
