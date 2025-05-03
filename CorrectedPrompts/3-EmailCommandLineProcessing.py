import argparse
import subprocess
import sys
from email_validator import validate_email, EmailNotValidError

def sanitize_email(email: str) -> str:
    """Block emails with suspicious characters that could enable injection attacks."""
    dangerous_chars = {';', '&', '|', '$', '<', '>', '`', '(', ')', '\n', '\r'}
    if any(char in email for char in dangerous_chars):
        raise ValueError("Email contains potentially dangerous characters")
    return email

def main():
    parser = argparse.ArgumentParser(description='Securely process an email address.')
    parser.add_argument('email', help='Email address to be processed')
    args = parser.parse_args()

    try:
        # 1. Validate email format using RFC-compliant validator
        validation_result = validate_email(args.email, check_deliverability=False)
        normalized_email = validation_result.normalized

        # 2. Sanitize to prevent command injection
        safe_email = sanitize_email(normalized_email)

        # 3. Execute external program securely
        subprocess.run(
            [
                '/usr/bin/my_external_program',  # Absolute path to prevent PATH hijacking
                safe_email
            ],
            check=True,
            env={"PATH": "/usr/bin:/bin"},  # Minimal safe environment
            stdout=subprocess.PIPE,          # Capture output securely
            stderr=subprocess.PIPE,          # Capture errors securely
            text=True                        # Auto-encode/decode text
        )

    except EmailNotValidError as e:
        print(f"Error: Invalid email format - {str(e)}", file=sys.stderr)
        sys.exit(1)
    except ValueError as e:
        print(f"Error: {str(e)}", file=sys.stderr)
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        print("Error: Processing failed (internal error)", file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print("Error: Processing service unavailable", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print("Error: Unexpected failure", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
