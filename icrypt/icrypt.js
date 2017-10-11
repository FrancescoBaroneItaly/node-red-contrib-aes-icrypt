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

    function ICryptNode(config) {
        RED.nodes.createNode(this,config);
        var node = this;

        this.on('input', function(msg) {

			var str = msg.payload;

			var key;
			if (typeof this.credentials.key != 'undefined')key = this.credentials.key;
			if (typeof msg.pairkey != 'undefined'){

				key = msg.pairkey;
				msg.pairkey="";
				}

			msg.rc=0;

			//console.log("CRYPT KEY = "+key);
			if (typeof key == 'undefined' || key==""){

				msg.rc=-2;
				msg.payload=-2;

			}else{

				msg.payload = [];
				/*
				AUTO convert #future
				var type_encrypt_data = xxtea.encrypt(xxtea.toBytes(typeof str), xxtea.toBytes(key));
				msg.payload.push(new Buffer(type_encrypt_data).toString('base64'));
				*/

				if (typeof str === 'object' || str instanceof Object){

					str = JSON.stringify(str);
					}

				if (typeof str === 'boolean' || str instanceof Boolean){

					str = str.toString();
					}

				if (typeof str === 'number' || str instanceof Number){

					str = str.toString();
					}

				//--generate IV
				var geniv = String(CryptoJS.lib.WordArray.random(16));
				var iv = CryptoJS.enc.Hex.parse(geniv);
				//console.log("GEN IV="+iv);

				//--put b64
				var b64iv = new Buffer(geniv,'hex').toString('base64');
				//console.log("IV="+b64iv);

				//IV is the first member of array
				//msg.iv = String(b64iv);
				msg.payload.push(String(b64iv));

				//KEY
				var AESKey = toHex(key);
				var aeskey= CryptoJS.enc.Hex.parse( AESKey );

				console.log("KEY");
				console.log(AESKey);

				if(str.length>16){

					var i=0;
					for(i=0;i<(str.length-16);i+=16){

						var substr = str.substring(i,i+16);

						var b64data = new Buffer(substr).toString('base64');
						var encrypt_data = CryptoJS.AES.encrypt(b64data, aeskey , { iv: iv} );
						msg.payload.push(encrypt_data.toString(CryptoJS.enc.base64));
						}

					if( (str.length - i)>0){

						var substr = str.substring(i);

						var b64data = new Buffer(substr).toString('base64');
						var encrypt_data = CryptoJS.AES.encrypt(b64data, aeskey , { iv: iv} );
						msg.payload.push(encrypt_data.toString(CryptoJS.enc.base64));
						}

				}else{

					var b64data = new Buffer(str).toString('base64');
					var encrypt_data = CryptoJS.AES.encrypt(b64data, aeskey , { iv: iv} );
					msg.payload.push(encrypt_data.toString(CryptoJS.enc.base64));
					}
				}

            node.send(msg);
        	});
    }
    RED.nodes.registerType("icrypt",ICryptNode,{
        credentials: {
		    key: {type: "password"}
        	}
		});
}