import sys
import onnxruntime as ort
import argparse


def extract_io_info(model_path):
    """
    Extract input and output information from an ONNX model.

    Args:
        model_path (str): Path to the .onnx model file
    """
    try:
        # Create inference session
        session = ort.InferenceSession(model_path)

        # Get input details
        print("\nModel Inputs:")
        for input_detail in session.get_inputs():
            print(f"- Name: {input_detail.name}")
            print(f"  Shape: {input_detail.shape}")
            print(f"  Type: {input_detail.type}")

        # Get output details
        print("\nModel Outputs:")
        for output_detail in session.get_outputs():
            print(f"- Name: {output_detail.name}")
            print(f"  Shape: {output_detail.shape}")
            print(f"  Type: {output_detail.type}")

    except Exception as e:
        print(f"Error loading model: {str(e)}")
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description="Extract input/output information from ONNX model"
    )
    parser.add_argument("model_path", help="Path to the .onnx model file")
    args = parser.parse_args()

    extract_io_info(args.model_path)


if __name__ == "__main__":
    main()
