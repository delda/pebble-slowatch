var Clay = require('@rebble/clay');
var config = require('./config');

var watchInfo = Pebble.getActiveWatchInfo && Pebble.getActiveWatchInfo();
if (watchInfo && watchInfo.platform === 'flint') {
  config[1].defaultValue = 'bottom';
}

new Clay(config);
