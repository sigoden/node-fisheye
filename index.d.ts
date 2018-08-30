import { Buffer } from "buffer";
export type Matx33d = Array<Array<Number>>;
export type Vet4d = Array<Number>;

type Opaque<T, K> = T & { __opaque__: K };
type Int = Opaque<number, "Int">;

/**
 * Performs camera calibaration
 * @param images - The batch checkboard images used to calibrate.
 * @param checkboardWidth - The number of cells in horizontal of checkboard.
 * @param checkboardHeight - The number of cells in vertial of checkboard.
 */
export function calibrate(
  images: Buffer[],
  checkboardWidth: Int,
  checkboardHeight: Int
);

// Options to control the generation of undistorted image.
interface UndistortExtra {
  // Format of the dest image, use extname `.jpg`, `.png` to repersent
  ext?: string,
  /**
   * Quantity of the dest image
   * For JPEG, it can be a quality ( CV_IMWRITE_JPEG_QUALITY ) from 0 to 100 (the higher is the better). Default value is 95.
   * For PNG, it can be the compression level ( CV_IMWRITE_PNG_COMPRESSION ) from 0 to 9. A higher value means a smaller size and longer compression time. Default value is 3.
   * For WEBP, it can be a quality ( CV_IMWRITE_WEBP_QUALITY ) from 1 to 100 (the higher is the better). By default (without any parameter) and for quality above 100 the lossless compression is used.
   */
  quantity?: Int,
  // Scale of the dest image 
  scale?: number,
}

/**
 * Transforms an image to compensate for fisheye lens distortion.
 * @param image - The image to process
 * @param K - Camera matrix \f$K = \vecthreethree{f_x}{0}{c_x}{0}{f_y}{c_y}{0}{0}{_1}\f$.
 * @param D - Input vector of distortion coefficients \f$(k_1, k_2, k_3, k_4)\f$.
 * @param extra - Control how the undistorted image generated.
 */
export function undistort(image: Buffer, K: Matx33d, D: Vet4d, extra?: UndistortExtra);