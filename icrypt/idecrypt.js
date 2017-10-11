module.exports = function(RED) {
	"use strict";
	var CryptoJS = require("crypto-js");

	function toHex(str) {
	    var hex = '';
	    var i = 0;
	    while(str.length > i) {
	        hex += ''+str.charCodeAt(i).toString(16);
	        i++;
	    	}

	    var j=0;
		for(j=i;j<16;j++)hex+= '00';

	    return hex;
		}

    function IDeCryptNode(config) {
        RED.nodes.createNode(this,config);
        var node = this;

        this.on('input', function(msg) {

			var str_array = msg.payload;

			//var key = config.key;
			var key;
			if (typeof this.credentials.key != 'undefined')key = this.credentials.key;
			if (typeof msg.pairkey != 'undefined'){

				key = msg.pairkey;
				msg.pairkey="";
				}

			msg.rc=0;

			//console.log("DECRYPT KEY = "+key);
			if (typeof key == 'undefined'  || key=="" || str_array.length<1 ){

				msg.rc=-2;
				msg.payload=-2;

			}else{

				try {

					var msgiv = str_array[0].toString();
					var AESKey = toHex(key);
					var aeskey= CryptoJS.enc.Hex.parse( AESKey );

					console.log("KEY");
					console.log(AESKey);

					var dato_type = "";
					var dato = "";

					var arrayLength = str_array.length;
					//console.log(arrayLength);
					for (var i = 1; i < arrayLength; i++) {

						var encrypt_data = str_array[i].toString();
						//console.log("MEMBER="+encrypt_data);

						var plain_iv =  new Buffer( msgiv , 'base64').toString('hex');
						var iv = CryptoJS.enc.Hex.parse( plain_iv );

						var bytes  = CryptoJS.AES.decrypt( encrypt_data, aeskey , { iv: iv} );
						var plaintext = bytes.toString(CryptoJS.enc.Base64);

						var decoded_b64msg =  new Buffer(plaintext , 'base64').toString('ascii');
						var decrypt_data =  new Buffer( decoded_b64msg , 'base64').toString('ascii');
						//console.log(decrypt_data);

						dato=dato.concat(decrypt_data);
						/*
						AUTO convert #future
						if(i==0){
							dato_type=decrypt_data;
						}else{
							dato=dato.concat(decrypt_data);
							}
						*/
						}

					msg.payload = dato;
					/*
					AUTO convert #future
					if(dato_type == "boolean"){

						msg.payload = (dato.toLowerCase() === 'true');
						}

					if(dato_type == "string"){

						msg.payload = dato;
						}

					if(dato_type == "number"){

						msg.payload = Number(dato);
						}

					if(dato_type == "object"){

						msg.payload = JSON.parse(dato);
						}
					*/

				} catch (e) {

					msg.rc=-1;
					msg.payload = -1;
					console.log("I'm not able to decrypt data");
					console.log(e);
					//throw new Error('Invalid Key');
					}

				}
            //msg.payload = config.key;
            node.send(msg);
        	});
    }
    RED.nodes.registerType("idecrypt",IDeCryptNode,{
        credentials: {
		    key: {type: "password"}
        	}
		});
}