const path = require('path');
const { promisify } = require('util');
const fs = require('fs');

const CHECKBOARD_WIDTH = 9;
const CHECKBOARD_HEIGHT = 6;
const IMG_FOLDER = path.join(__dirname, 'samples');

const calibrate = require('./calibrate');
const undistort = require('./undistort');

async function run(src, dest) {
  let {K , D} = await calibrate(IMG_FOLDER, CHECKBOARD_WIDTH, CHECKBOARD_HEIGHT);
  let buf = await undistort(src, K, D);
  await promisify(fs.writeFile)(dest, buf);
}

if (process.argv.length !== 4) {
  console.log('Usage: fisheye <src> <dest>');
  process.exit(1);
}

run(process.argv[2], process.argv[3]);
