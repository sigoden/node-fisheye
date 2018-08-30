const fs = require('fs');
const fisheye = require('..');
let imgs = fs
    .readdirSync('example/samples')
    .map(file => fs.readFileSync('example/samples/'+file));
let {K, D} = fisheye.calibrate(imgs, 9, 6);
let img = fs.readFileSync('example/samples/IMG-0.jpg');
let buf = fisheye.undistort(img, K, D);
fs.writeFileSync('/tmp/IMG-0.jpg', buf);
