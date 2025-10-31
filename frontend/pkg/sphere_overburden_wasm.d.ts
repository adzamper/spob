/* tslint:disable */
/* eslint-disable */
/**
 * Main WASM entry point for electromagnetic calculation
 *
 * Computes the complete EM response along a 2D survey profile for all time windows.
 * This is the function called from JavaScript.
 *
 * # Arguments
 * * `params_json` - JSON string containing all survey parameters
 *
 * # Returns
 * ResponseData containing field responses at 201 profile positions and 15 time windows
 */
export function calculate_em_response(params_json: string): ResponseData;
/**
 * Complete electromagnetic response data structure
 * Contains field responses at all profile positions and time windows
 */
export class ResponseData {
  private constructor();
  free(): void;
  [Symbol.dispose](): void;
  /**
   * Export response data as JSON string for JavaScript consumption
   */
  toJSON(): string;
}
/**
 * 3D vector structure for JavaScript interface
 * Represents positions, offsets, and field directions in 3D space
 */
export class Vector3 {
  free(): void;
  [Symbol.dispose](): void;
  constructor(x: number, y: number, z: number);
  x: number;
  y: number;
  z: number;
}

export type InitInput = RequestInfo | URL | Response | BufferSource | WebAssembly.Module;

export interface InitOutput {
  readonly memory: WebAssembly.Memory;
  readonly __wbg_vector3_free: (a: number, b: number) => void;
  readonly __wbg_get_vector3_x: (a: number) => number;
  readonly __wbg_set_vector3_x: (a: number, b: number) => void;
  readonly __wbg_get_vector3_y: (a: number) => number;
  readonly __wbg_set_vector3_y: (a: number, b: number) => void;
  readonly __wbg_get_vector3_z: (a: number) => number;
  readonly __wbg_set_vector3_z: (a: number, b: number) => void;
  readonly vector3_new: (a: number, b: number, c: number) => number;
  readonly __wbg_responsedata_free: (a: number, b: number) => void;
  readonly responsedata_toJSON: (a: number) => [number, number, number, number];
  readonly calculate_em_response: (a: number, b: number) => [number, number, number];
  readonly __wbindgen_externrefs: WebAssembly.Table;
  readonly __externref_table_dealloc: (a: number) => void;
  readonly __wbindgen_free: (a: number, b: number, c: number) => void;
  readonly __wbindgen_malloc: (a: number, b: number) => number;
  readonly __wbindgen_realloc: (a: number, b: number, c: number, d: number) => number;
  readonly __wbindgen_start: () => void;
}

export type SyncInitInput = BufferSource | WebAssembly.Module;
/**
* Instantiates the given `module`, which can either be bytes or
* a precompiled `WebAssembly.Module`.
*
* @param {{ module: SyncInitInput }} module - Passing `SyncInitInput` directly is deprecated.
*
* @returns {InitOutput}
*/
export function initSync(module: { module: SyncInitInput } | SyncInitInput): InitOutput;

/**
* If `module_or_path` is {RequestInfo} or {URL}, makes a request and
* for everything else, calls `WebAssembly.instantiate` directly.
*
* @param {{ module_or_path: InitInput | Promise<InitInput> }} module_or_path - Passing `InitInput` directly is deprecated.
*
* @returns {Promise<InitOutput>}
*/
export default function __wbg_init (module_or_path?: { module_or_path: InitInput | Promise<InitInput> } | InitInput | Promise<InitInput>): Promise<InitOutput>;
