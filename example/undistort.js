const fs = require("fs");
const fisheye = require("..");
const path = require("path");
const { promisify } = require("util");

async function undistort(img, K, D) {
  let originImg = await promisify(fs.readFile)(img);
  return fisheye.undistort(originImg, K, D, {
    ext: path.extname(img),
    scale: 1
  });
}

module.exports = undistort;