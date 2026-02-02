import json

def analyze(signals):
    """
    Sample analysis function.
    Args:
        signals (dict): The signals to analyze, passed from OCaml.
    Returns:
        dict: A dictionary containing {success, message, products}.
    """
    print(f"Python analysis invoked with signals: {signals}")
    
    # Simple logic for verification
    success = True
    message = "Python Analysis Successful"
    products = {
        "analysis_result": 42.0,
        "status": "processed",
        "original_signals_count": len(signals) if isinstance(signals, (dict, list)) else 0
    }
    
    return {
        "success": success,
        "message": message,
        "products": products
    }
