/* tslint:disable */
/* eslint-disable */
/**
 * TypeScript definitions for sphere_overburden_wasm
 * Generated from C++ source using Emscripten
 */

/**
 * Main WASM entry point for electromagnetic calculation
 *
 * Computes the complete EM response along a 2D survey profile for all time windows.
 * This is the function called from JavaScript.
 *
 * @param params_json - JSON string containing all survey parameters
 * @returns JSON string containing field responses at profile positions and time windows
 *
 * The returned JSON has the following structure:
 * {
 *   "x_values": number[],           // Profile positions (meters)
 *   "time_windows": number[],        // Time gates (seconds)
 *   "x_components": number[][],      // X-component responses [time][position]
 *   "y_components": number[][],      // Y-component responses [time][position]
 *   "z_components": number[][]       // Z-component responses [time][position]
 * }
 */
export function calculate_em_response(params_json: string): string;

/**
 * Emscripten module initialization type
 */
export type InitInput = RequestInfo | URL | Response | BufferSource | WebAssembly.Module;

/**
 * Emscripten module interface after initialization
 */
export interface EmscriptenModule {
  readonly memory: WebAssembly.Memory;
  calculate_em_response(params_json: string): string;
}

/**
 * Default export: Initialize the WASM module
 *
 * @param module_or_path - Optional path to WASM file or module
 * @returns Promise that resolves to the initialized module
 *
 * @example
 * ```typescript
 * import createModule from './sphere_overburden_wasm.js';
 *
 * const wasmModule = await createModule();
 * const response = wasmModule.calculate_em_response('{"param": "value"}');
 * const data = JSON.parse(response);
 * ```
 */
export default function createModule(module_or_path?: InitInput | Promise<InitInput>): Promise<EmscriptenModule>;
